/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2013-2015 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details.
 */

#include "harry_classes.h"

/**
 * @addtogroup measures
 * <hr>
 * <em>dist_bag</em>: Bag distance for strings.
 *
 * Bartolini, Ciaccia, Patella. String Matching with Metric Trees Using an
 * Approximate Distance.  String Processing and Information Retrieval, LNCS
 * 2476, 271-283, 2002.
 *
 */

/* Hash table */
typedef struct
{
    sym_t sym;          /**< Symbol or character */
    float cnt;          /**< Count of symbol */
    UT_hash_handle hh;  /**< uthash handle */
} bag_t;

/**
 * Initializes the similarity measure
 */
void
dist_bag_config (measures_t *self)
{
    assert (self);
    const char *str;
    measures_opts_t *opts = self->opts;

    //  Apply normalization
    config_lookup_string (self->cfg, "measures.dist_bag.norm", &str);
    opts->lnorm = lnorm_get (str);
}

/**
 * Computes a histogram of symbols or characters
 * @param x string
 * @return histogram
 */
static bag_t *
bag_new (hstring_t *x)
{
    bag_t *xh = NULL, *bag = NULL;

    for (int i = 0; i < x->len; i++) {
        sym_t s = hstring_get(x, i);
        HASH_FIND(hh, xh, &s, sizeof(sym_t), bag);

        if (!bag) {
            bag = (bag_t *) zmalloc(sizeof(bag_t));
            bag->sym = s;
            bag->cnt = 0;
            HASH_ADD(hh, xh, sym, sizeof(sym_t), bag);
        }

        bag->cnt++;
    }

    return xh;
}

/**
 * Free the memory of histogram
 * @param xh Histogram
 */
static void bag_destroy(bag_t * xh)
{
    /* Clear hash table */
    while (xh) {
        bag_t *bag = xh;
        HASH_DEL(xh, bag);
        free(bag);
    }
}

/**
 * Computes the bag distance of two strings. The distance approximates
 * and lower bounds the Levenshtein distance.
 * @param x first string
 * @param y second string
 * @return Bag distance
 */
float
dist_bag_compare (measures_t *self, hstring_t *x, hstring_t *y)
{
    assert (self);
    float xd = 0, yd = 0;
    bag_t *xh, *yh, *xb, *yb;
    measures_opts_t *opts = self->opts;

    xh = bag_new (x);
    yh = bag_new (y);

    int missing = y->len;
    for (xb = xh; xb != NULL; xb = (bag_t *) xb->hh.next) {
        HASH_FIND(hh, yh, &(xb->sym), sizeof(sym_t), yb);
        if (!yb) {
            xd += xb->cnt;
        } else {
            float diff = xb->cnt - yb->cnt;
            xd += fmax(+diff, 0);
            yd += fmax(-diff, 0);
            missing -= yb->cnt;
        }
    }
    yd += missing;

    bag_destroy (xh);
    bag_destroy (yh);

    return lnorm (opts->lnorm, fmax(xd, yd), x, y);
}

//  --------------------------------------------------------------------------
//  Self test of this class

/*
 * Structure for testing string kernels/distances
 */
struct hstring_test
{
    char *x;            /**< String x */
    char *y;            /**< String y */
    char *delim;        /**< Delimiter string */
    float v;            /**< Expected output */
};

struct hstring_test tests[] = {
    /* Comparison using bytes */
    {"", "", "", 0},
    {"a", "", "", 1},
    {"", "a", "", 1},
    {"a", "a", "", 0},
    {"ab", "ba", "", 0},
    {"bab", "ba", "", 1},
    {"abba", "babb", "", 1},
    {"a.b", "a.c", "", 1},
    {".a.b.", "a..c.", "", 1},
    /* Comparison using tokens */
    {"", "", ".", 0},
    {"a", "", ".", 1},
    {"", "a", ".", 1},
    {"a", "a", ".", 0},
    {"ab", "ba", ".", 1},
    {"bab", "ba", ".", 1},
    {"abba", "babb", ".", 1},
    {"a.b", "a.c", ".", 1},
    {".a.b.", "a..c.", ".", 1},
    /* Further test cases */
    {"abcd", "axcy", "", 2},
    {"abc", "axcy", "", 2},
    {"abcd", "xcy", "", 3},
    {".x.y.", ".x.y.", ".", 0},
    {"x...y..", "...x..y", ".", 0},
    {".x.y", "x.y.", ".", 0},
    /* Examples from paper by Bartolini et al. */
    {"spire", "fare", "", 3},
    {"fare", "spire", "", 3},
    {"spire", "paris", "", 1},
    {"paris", "spire", "", 1},
    {NULL}
};

void
dist_bag_test (bool verbose)
{
    printf("    Bag distance:");

    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *bag = measures_new ("dist_bag");
    assert (bag);

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        if (strlen(tests[i].delim) == 0)
            measures_config_set_string (bag, "measures.granularity", "bytes");
        else
            measures_config_set_string (bag, "measures.granularity", "tokens");

        hstring_delim_set (tests[i].delim);

        hstring_preproc (x, bag);
        hstring_preproc (y, bag);

        float d = measures_compare (bag, x, y);
        double diff = fabs(tests[i].v - d);

        if (diff > 1e-6) {
            printf ("Error %f != %f\n", d, tests[i].v);
            hstring_print (x);
            hstring_print (y);
            err = TRUE;
        }

        hstring_destroy(&x);
        hstring_destroy(&y);
    }
    measures_destroy (&bag);
    //  @end

    printf(" OK\n");
}
