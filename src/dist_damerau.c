/*  =========================================================================
    dist_damerau - implementation of Damerau-Levenshtein edit distance

    Copyright (C) 2006 Stephen Toub
                  2013-2015  Konrad Rieck (konrad@mlsec.org)

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.  This program is distributed without any
    warranty. See the GNU General Public License for more details.
    =========================================================================
*/

/*
@header
    dist_damerau: Damerau-Levenshtein distance for strings.

    Damerau. A technique for computer detection and correction of spelling
    errors, Communications of the ACM, 7(3):171-176, 1964
@end
*/

#include "harry_classes.h"

//  Symbols hash table
typedef struct
{
    sym_t sym;          /**< Symbol (key) */
    int val;            /**< Value associated with symbol */
    UT_hash_handle hh;  /**< Makes struct hashable */
} sym_hash_t;

//  Local helper functions

/*
 * Four-way minimum
 */
static float min(float a, float b, float c, float d)
{
    return fmin(fmin(a, b), fmin(c, d));
}

/**
 * Get value associated with a symbol from the hash
 * @param hash hash table
 * @param s symbol
 * @return value
 */
static int hash_get(sym_hash_t **hash, sym_t s)
{
    sym_hash_t *entry = NULL;

    HASH_FIND(hh, *hash, &s, sizeof(sym_t), entry);
    if (!entry) {
        entry = (sym_hash_t *) zmalloc(sizeof(sym_hash_t));
        entry->sym = s;
        entry->val = 0;
        HASH_ADD(hh, *hash, sym, sizeof(sym_t), entry);
    }

    return entry->val;
}

/**
 * Set the value of a symbol in the hash
 * @param hash hash table
 * @param s symbol
 * @param val value
 */
static void hash_set(sym_hash_t **hash, sym_t s, int val)
{
    sym_hash_t *entry = NULL;

    HASH_FIND(hh, *hash, &s, sizeof(sym_t), entry);
    if (!entry) {
        entry = (sym_hash_t *) zmalloc(sizeof(sym_hash_t));
        entry->sym = s;
        entry->val = val;
        HASH_ADD(hh, *hash, sym, sizeof(sym_t), entry);
    }

    entry->val = val;
}

/**
 * Destroy hash table
 * @param hash hash table
 */
static void hash_destroy(sym_hash_t **hash)
{
    sym_hash_t *entry;

    while (*hash) {
        entry = *hash;
        HASH_DEL(*hash, entry);
        free(entry);
    }
}

//  --------------------------------------------------------------------------
//  Initializes the similarity measure

void
dist_damerau_config (measures_t *self)
{
    assert (self);
    const char *str;
    measures_opts_t *opts = self->opts;

    //  Apply costs
    config_lookup_float(self->cfg, "measures.dist_damerau.cost_ins", &opts->cost_ins);
    config_lookup_float(self->cfg, "measures.dist_damerau.cost_del", &opts->cost_del);
    config_lookup_float(self->cfg, "measures.dist_damerau.cost_sub", &opts->cost_sub);
    config_lookup_float(self->cfg, "measures.dist_damerau.cost_tra", &opts->cost_tra);
    //  Apply normalization
    config_lookup_string(self->cfg, "measures.dist_damerau.norm", &str);
    opts->lnorm = lnorm_get(str);
}

/* Ugly macros to access arrays */
#define D(i,j)       d[(i) * (y->len + 2) + (j)]

//  --------------------------------------------------------------------------
//  Computes the Damerau-Levenshtein distance of two strings. Adapted from
//  Wikipedia entry and comments from Stackoverflow.com. Takes two strings and
//  returns the edit distance consisting of insertions, deletions, replacements
//  and transpositions weighted by costs in the configuration by default cost
//  for each operation is 1.0. @TODO normalizations

float
dist_damerau_compare (measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    sym_hash_t *shash = NULL;
    int i, j, inf = x->len + y->len;

    if (x->len == 0 && y->len == 0)
        return 0;

    /* Allocate table for dynamic programming */
    int *d = (int *) zmalloc((x->len + 2) * (y->len + 2) * sizeof(int));
    if (!d) {
        error("Could not allocate memory for Damerau-Levenshtein distance");
        return 0;
    }

    /* Initialize distance matrix */
    D(0, 0) = inf;
    for (i = 0; i <= x->len; i++) {
        D(i + 1, 1) = i;
        D(i + 1, 0) = inf;
    }
    for (j = 0; j <= y->len; j++) {
        D(1, j + 1) = j;
        D(0, j + 1) = inf;
    }

    for (i = 1; i <= x->len; i++) {
        int db = 0;
        for (j = 1; j <= y->len; j++) {
            int i1 = hash_get(&shash, hstring_get(y, j - 1));
            int j1 = db;
            int dz = hstring_compare(x, i - 1, y, j - 1) ? opts->cost_sub : 0;
            if (dz == 0)
                db = j;

            D(i + 1, j + 1) = min(D(i, j) + dz,
                                  D(i + 1, j) + opts->cost_ins,
                                  D(i, j + 1) + opts->cost_del,
                                  D(i1, j1) + (i - i1 - 1) + opts->cost_tra +
                                  (j - j1 - 1));
        }

        hash_set(&shash, hstring_get(x, i - 1), i);
    }

    float r = D(x->len + 1, y->len + 1);

    /* Free memory */
    free(d);
    hash_destroy(&shash);

    if (opts->lnorm == LN_NONE)
        return r;
    else
    if (fabs (opts->cost_ins - opts->cost_del) < 1e-6
    &&  fabs (opts->cost_del - opts->cost_sub) < 1e-6
    &&  fabs (opts->cost_sub - opts->cost_tra) < 1e-6) {
        double w = fmax(fmax(fmax(opts->cost_ins, opts->cost_del), opts->cost_sub), opts->cost_tra);
        return 1 - wlnorm(opts->lnorm, r, w, x, y);
    }
    else
        return 1 - lnorm(opts->lnorm, r, x, y);
}


//  --------------------------------------------------------------------------
//  Self test of this class

// Structure for testing string kernels/distances
struct hstring_test
{
    char *x;            /**< String x */
    char *y;            /**< String y */
    float v;            /**< Expected output */
};

static struct hstring_test tests[] = {
    /* Comparison using bytes */
    {"", "", 0},
    {"a", "", 1},
    {"", "a", 1},
    {"a", "a", 0},
    {"pantera", "aorta", 4},
    {"ca", "abc", 2},
	{"transpose", "tranpsose", 1},
    {"Healed", "Sealed", 1},
    {"Healed", "Healthy", 3},
    {"Healed", "Heard", 2},
    {"Healed", "Herded", 2},
    {"Healed", "Help", 3},
    {"Healed", "Sold", 4},
    {"Healed", "Help", 3},
    {"Sam J Chapman", "Samuel John Chapman", 6},
    {"Sam Chapman", "S Chapman", 2},
    {"John Smith", "Samuel John Chapman", 14},
    {"John Smith", "Sam Chapman", 11},
    {"John Smith", "Sam J Chapman", 12},
    {"John Smith", "S Chapman", 9},
    {"Web Database Applications", "Web Database Applications with PHP & MySQL", 17},
    {"Web Database Applications", "Creating Database Web Applications with PHP and ASP", 28},
    {"Web Database Applications", "Building Database Applications on the Web Using PHP3", 30},
    {"Web Database Applications", "Building Web Database Applications with Visual Studio 6", 30},
    {"Web Database Applications", "Web Application Development With PHP", 27},
    {"Web Database Applications", "WebRAD: Building Database Applications on the Web with Visual FoxPro and Web Connection", 62},
    {"Web Database Applications", "Structural Assessment: The Role of Large and Full-Scale Testing", 53},
    {"Web Database Applications", "How to Find a Scholarship Online", 27},
    {"Web Aplications", "Web Database Applications with PHP & MySQL", 27},
    {"Web Aplications", "Creating Database Web Applications with PHP and ASP", 36},
    {"Web Aplications", "Building Database Applications on the Web Using PHP3", 39},
    {"Web Aplications", "Building Web Database Applications with Visual Studio 6", 40},
    {"Web Aplications", "Web Application Development With PHP", 22},
    {"Web Aplications", "WebRAD: Building Database Applications on the Web with Visual FoxPro and Web Connection", 72},
    {"Web Aplications", "Structural Assessment: The Role of Large and Full-Scale Testing", 56},
    {"Web Aplications", "How to Find a Scholarship Online", 26},
    {NULL}
};

void
dist_damerau_test (bool verbose)
{
    printf(" * Damerau-Levenshtein distance:");

    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *damerau = measures_new ("dist_damerau");

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        hstring_preproc (x, damerau);
        hstring_preproc (y, damerau);

        float d = measures_compare (damerau, x, y);
        double diff = fabs (tests[i].v - d);

        if (diff > 1e-6) {
            printf ("Error %f != %f\n", d, tests[i].v);
            hstring_print (x);
            hstring_print (y);
            err = TRUE;
        }

        hstring_destroy (&x);
        hstring_destroy (&y);
    }
    measures_destroy (&damerau);
    //  @end

    printf(" OK\n");
}
