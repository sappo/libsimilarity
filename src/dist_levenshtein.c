/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2006 Stephen Toub
 * Copyright (C) 2002-2003 David Necas (Yeti) <yeti@physics.muni.cz>.
 * Copyright (C) 2013-2015  Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it * under the terms of the GNU General Public License as published by the * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details.
 */

#include "harry_classes.h"

/**
 * @addtogroup measures
 * <hr>
 * <em>dist_levenshtein</em>: Levenshtein distance for strings.
 *
 * Levenshtein. Binary codes capable of correcting deletions, insertions,
 * and reversals. Doklady Akademii Nauk SSSR, 163 (4):845-848, 1966.
 * @{
 */

/**
 * Initializes the similarity measure
 */
void
dist_levenshtein_config (measures_t *self)
{
    assert (self);
    const char *str;
    measures_opts_t *opts = self->opts;

    //  Apply costs
    config_lookup_float(self->cfg, "measures.dist_levenshtein.cost_ins",
                        &opts->cost_ins);
    config_lookup_float(self->cfg, "measures.dist_levenshtein.cost_del",
                        &opts->cost_del);
    config_lookup_float(self->cfg, "measures.dist_levenshtein.cost_sub",
                        &opts->cost_sub);
    //  Apply normalization
    config_lookup_string(self->cfg, "measures.dist_levenshtein.norm", &str);
    opts->lnorm = lnorm_get(str);
}




/**
 * Computes the Levenshtein distance. Code adapted from
 * David Necas (Yeti) <yeti@physics.muni.cz>.
 * @param x first string
 * @param y second string
 * @return Levenshtein distance
 */
static float
dist_levenshtein_compare_yeti(hstring_t *x, hstring_t *y)
{
    int i, *end, half;
    int *row; /* we only need to keep one row of costs */

    /* Catch trivial cases */
    if (x->len == 0)
        return y->len ;
    if (y->len  == 0)
        return x->len;

    /* Make the inner cycle (i.e. string2) the longer one */
    if (x->len > y->len ) {
        hstring_t *z = x;
        x = y;
        y = z;
    }

    /* Check x->len == 1 separately */
    if (x->len == 1) {
	int c = 0;
	for (int k = 0; !c && k < y->len; k++) {
	    if (!hstring_compare(x, 0, y, k))
		c = 1;
	}
	return y->len - c;
    }

    x->len++;
    y->len++;
    half = x->len >> 1;

    /* Unitalize first row */
    row = (int *) zmalloc ((y->len) * sizeof (int));
    if (!row) {
        error("Failed to allocate memory for Levenshtein distance");
        return 0;
    }

    end = row + y->len  - 1;
    for (i = 0; i < y->len  - half; i++)
        row[i] = i;

    /*
     * We don't have to scan two corner triangles (of size x->len/2) in the
     * matrix because no best path can go throught them.  Note this breaks
     * when x->len == y->len == 2 so special case above is necessary
     */
    row[0] = x->len  - half - 1;
    for (i = 1; i < x->len ; i++) {
        int *p;
        int char1p = i - 1;
        int char2p;
        int D, k;
        /* skip the upper triangle */
        if (i >= x->len  - half) {
            int offset = i - (x->len  - half);
            int c3;

            char2p = offset;
            p = row + offset;
            c3 = *(p++) + (hstring_compare(x, char1p, y, char2p++) ? 1 : 0);
            k = *p;
            k++;
            D = k;
            if (k > c3)
                k = c3;
            *(p++) = k;
        } else {
            p = row + 1;
            char2p = 0;
            D = k = i;
        }
        /* skip the lower triangle */
        if (i <= half + 1)
            end = row + y->len  + i - half - 2;
        /* main */
        while (p <= end) {
            int c3 = --D + (hstring_compare(x, char1p, y, char2p++) ? 1 : 0);
            k++;
            if (k > c3)
                k = c3;
            D = *p;
            D++;
            if (k > D)
                k = D;
            *(p++) = k;
        }
        /* lower triangle sentinel */
        if (i <= half) {
            int c3 = --D + (hstring_compare(x, char1p, y, char2p) ? 1 : 0);
            k++;
            if (k > c3)
                k = c3;
            *p = k;
        }
    }

    i = *end;
    free(row);
    return i;
}

/* Ugly macros to access arrays */
#define ROWS(i,j)	rows[(i) * (y->len + 1) + (j)]

/**
 * Computes the Levenshtein distance of two strings.
 * Adapted from Stephen Toub's C# implementation.
 * http://blogs.msdn.com/b/toub/archive/2006/05/05/590814.aspx
 * @param x first string
 * @param y second string
 * @return Levenshtein distance
 */
static float
dist_levenshtein_compare_toub (measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    int i, j;
    double a, b;

    if (x->len == 0 && y->len == 0)
        return 0;

    /*
     * Rather than maintain an entire matrix (which would require O(n*m)
     * space), just store the current row and the next row, each of which
     * has a length m+1, so just O(m) space.  Initialize the curr row.
     */
    int curr = 0, next = 1;
    double *rows = (double *) zmalloc(sizeof(double) * (y->len + 1) * 2);
    if (!rows) {
        error("Failed to allocate memory for Levenshtein distance");
        return 0;
    }

    for (j = 0; j <= y->len; j++)
         ROWS(curr,j) = j;

    /* For each virtual row (we only have physical storage for two) */
    for (i = 1; i <= x->len; i++) {

        /* Fill in the values in the row */
        ROWS(next,0) = i;
        for (j = 1; j <= y->len; j++) {

            /* Insertion and deletion */
            a = ROWS(curr,j) + opts->cost_ins;
            b = ROWS(next, j - 1) + opts->cost_del;
            if (a > b)
                a = b;

            /* Substitution */
            b = ROWS(curr, j - 1) +
                (hstring_compare(x, i - 1, y, j - 1) ? opts->cost_sub : 0);

            if (a > b)
                a = b;

            /*
             * Transpositions (Damerau-Levenshtein) are not supported by
             * this implementation, as only two rows of the distance matrix
             * are available. Potential fix: provide three rows.
             */
            ROWS(next, j) = a;
        }

        /* Swap the current and next rows */
        if (curr == 0) {
            curr = 1;
            next = 0;
        } else {
            curr = 0;
            next = 1;
        }
    }
    double d = ROWS(curr, y->len);

    /* Free memory */
    free(rows);

    return d;
}


/**
 * Computes the Levenshtein distance. Wrapper function.
 * @param x first string
 * @param y second string
 * @return Levenshtein distance
 */
float
dist_levenshtein_compare (measures_t *self, hstring_t *x, hstring_t *y)
{
    assert (self);
    float f;
    measures_opts_t *opts = self->opts;

    /*
     * If the costs of all edit operations are equal we use the fast
     * implementation by David Necas, otherwise we switch to the
     * variant by Stephen Toub.
     */
    if (fabs (opts->cost_ins - opts->cost_del) < 1e-6
     && fabs (opts->cost_del - opts->cost_sub) < 1e-6) {
        f = opts->cost_ins * dist_levenshtein_compare_yeti (x, y);
    } else {
        f = dist_levenshtein_compare_toub (self, x, y);
    }

    return lnorm (opts->lnorm, f, x, y);
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

static struct hstring_test tests[] = {
    /* Comparison using bytes */
    {"", "", "", 0},
    {"a", "", "", 1},
    {"", "a", "", 1},
    {"a", "a", "", 0},
    {"ab", "ba", "", 2},
    {"bab", "ba", "", 1},
    {"abba", "babb", "", 2},
    {"a.b", "a.c", "", 1},
    {".a.b.", "a..c.", "", 3},
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
    /* Tests for new implementation */
    {"a", "b", "", 1},
    {"aa", "aa", "", 0},
    {"ab", "aa", "", 1},
    {"aba", "aaa", "", 1},
    {"a", "bab", "", 2},
    {"bbb", "a", "", 3},
    {"yyybca", "yyycba", "", 2},
    {"bcaxxx", "cbaxxx", "", 2},
    {"yyybcaxxx", "yyycbaxxx", "", 2},
    /* Simetrics test cases */
    {"Healed", "Sealed", "", 1},
    {"Healed", "Healthy", "", 3},
    {"Healed", "Heard", "", 2},
    {"Healed", "Herded", "", 2},
    {"Healed", "Help", "", 3},
    {"Healed", "Sold", "", 4},
    {"Healed", "Help", "", 3},
    {"Sam J Chapman", "Samuel John Chapman", "", 6},
    {"Sam Chapman", "S Chapman", "", 2},
    {"John Smith", "Samuel John Chapman", "", 14},
    {"John Smith", "Sam Chapman", "", 11},
    {"John Smith", "Sam J Chapman", "", 12},
    {"John Smith", "S Chapman", "", 9},
    {"Web Database Applications", "Web Database Applications with PHP & MySQL", "", 17},
    {"Web Database Applications", "Creating Database Web Applications with PHP and ASP", "", 28},
    {"Web Database Applications", "Building Database Applications on the Web Using PHP3", "", 30},
    {"Web Database Applications", "Building Web Database Applications with Visual Studio 6", "", 30},
    {"Web Database Applications", "Web Application Development With PHP", "", 27},
    {"Web Database Applications", "WebRAD: Building Database Applications on the Web with Visual FoxPro and Web Connection", "", 62},
    {"Web Database Applications", "Structural Assessment: The Role of Large and Full-Scale Testing", "", 53},
    {"Web Database Applications", "How to Find a Scholarship Online", "", 27},
    {"Web Aplications", "Web Database Applications with PHP & MySQL", "", 27},
    {"Web Aplications", "Creating Database Web Applications with PHP and ASP", "", 36},
    {"Web Aplications", "Building Database Applications on the Web Using PHP3", "", 39},
    {"Web Aplications", "Building Web Database Applications with Visual Studio 6", "", 40},
    {"Web Aplications", "Web Application Development With PHP", "", 22},
    {"Web Aplications", "WebRAD: Building Database Applications on the Web with Visual FoxPro and Web Connection", "", 72},
    {"Web Aplications", "Structural Assessment: The Role of Large and Full-Scale Testing", "", 56},
    {"Web Aplications", "How to Find a Scholarship Online", "", 26},
    {NULL}
};

/*
 * Structure for testing string kernels/distances
 */
struct hstring_test_weighted
{
    char *x;            /**< String x */
    char *y;            /**< String y */
    char *delim;        /**< Delimiter string */
    float v;            /**< Expected output */
    double cost_ins;
    double cost_del;
    double cost_sub;
};

static struct hstring_test_weighted weighted_tests[] = {
    {"abc", "ab", "", 1, 1, 1, 1},
    {"abc", "ab", "", 2, 2, 1, 1},
    {"abc", "ab", "", 3, 3, 1, 1},
    {"ab", "abc", "", 1, 1, 1, 1},
    {"ab", "abc", "", 2, 1, 2, 1},
    {"ab", "abc", "", 3, 1, 3, 1},
    {"abc", "adc", "", 1, 1, 1, 1},
    {"abc", "adc", "", 2, 1, 1, 2}, // Substitution d -> b
    {"abc", "adc", "", 2, 1, 1, 3}, // Delete d + Insert b
    {"abc", "adc", "", 3, 1, 3, 3}, // Substitution d -> b
    {"abc", "adc", "", 3, 3, 1, 3}, // Substitution d -> b
    {"abc", "adc", "", 6, 4, 2, 15}, // Delete d + Insert b
    {"abc", "adc", "", 4, 2.5, 1.5, 15}, // Delete d + Insert b
    {NULL}
};


void
dist_levenshtein_test (bool verbose)
{
    printf(" * Levenshtein distance:");

    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *levenshtein = measures_new ("dist_levenshtein");

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        if (strlen(tests[i].delim) == 0)
            measures_config_set_string (levenshtein, "measures.granularity", "bytes");
        else
            measures_config_set_string (levenshtein, "measures.granularity", "tokens");

        hstring_delim_set (tests[i].delim);

        hstring_preproc (x, levenshtein);
        hstring_preproc (y, levenshtein);

        float d = measures_compare (levenshtein, x, y);
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

    for (i = 0; weighted_tests[i].x && !err; i++) {
        x = hstring_new (weighted_tests[i].x);
        y = hstring_new (weighted_tests[i].y);

        if (strlen(weighted_tests[i].delim) == 0)
            measures_config_set_string (levenshtein, "measures.granularity", "bytes");
        else
            measures_config_set_string (levenshtein, "measures.granularity", "tokens");

        measures_config_set_float (levenshtein, "measures.dist_levenshtein.cost_ins", weighted_tests[i].cost_ins);
        measures_config_set_float (levenshtein, "measures.dist_levenshtein.cost_del", weighted_tests[i].cost_del);
        measures_config_set_float (levenshtein, "measures.dist_levenshtein.cost_sub", weighted_tests[i].cost_sub);

        hstring_delim_set (weighted_tests[i].delim);

        hstring_preproc (x, levenshtein);
        hstring_preproc (y, levenshtein);

        float d = measures_compare (levenshtein, x, y);
        double diff = fabs (weighted_tests[i].v - d);

        if (diff > 1e-6) {
            printf ("Error %f != %f\n", d, weighted_tests[i].v);
            hstring_print (x);
            hstring_print (y);
            err = TRUE;
        }

        hstring_destroy (&x);
        hstring_destroy (&y);
    }
    measures_destroy (&levenshtein);
    //  @end

    printf(" OK\n");
}
