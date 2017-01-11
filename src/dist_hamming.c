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
 * <em>dist_hamming</em>: Hamming distance for strings.
 *
 * Hamming. Error-detecting and error-correcting codes. Bell System
 * Technical Journal, 29(2):147-160, 1950.
 * @{
 */

/**
 * Initializes the similarity measure
 */
void dist_hamming_config(measures_t *self)
{
    assert (self);
    measures_opts_t *opts = self->opts;
    const char *str;

    //  Apply normalization
    config_lookup_string(self->cfg, "measures.dist_hamming.norm", &str);
    opts->lnorm = lnorm_get(str);
}

/**
 * Computes the Hamming distance of two strings. If the strings have
 * different lengths, the remaining symbols of the longer string are
 * considered mismatches.
 * @param x first string
 * @param y second string
 * @return Hamming distance
 */
float dist_hamming_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    float d = 0;
    int i;

    /* Loop over strings */
    for (i = 0; i < x->len && i < y->len; i++)
        if (hstring_compare(x, i, y, i))
            d += 1;

    /* Add remaining characters as mismatches */
    d += fabs(y->len - x->len);

    return lnorm(opts->lnorm, d, x, y);
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
    {"abba", "babb", "", 3},
    {"a.b", "a.c", "", 1},
    {".a.b.", "a..c.", "", 3},
    // Simmetrics
    {"test 1", "test 1", "", 0},
    {"test 1", "test 2", "", 1},
    {"aaabbb", "aaaaaa", "", 3},
    {"abcdxy", "abcexy", "", 1},
    {"abcdxy", "abfexy", "", 2},
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
    {"abcd", "xcy", "", 4},
    {".x.y.", ".x.y.", ".", 0},
    {"x...y..", "...x..y", ".", 0},
    {".x.y", "x.y.", ".", 0},
    {NULL}
};

void
dist_hamming_test (bool verbose)
{
    printf (" * Hamming distance:");
    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *hamming = measures_new ("dist_hamming");
    assert (hamming);

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        if (strlen(tests[i].delim) == 0)
            measures_config_set_string(hamming, "measures.granularity", "bytes");
        else
            measures_config_set_string(hamming, "measures.granularity", "tokens");
        hstring_delim_set (tests[i].delim);

        hstring_preproc (x, hamming);
        hstring_preproc (y, hamming);

        float d = measures_compare(hamming, x, y);
        double diff = fabs (tests[i].v - d);

        if (diff > 1e-6) {
            printf("\nError %f != %f\n", d, tests[i].v);
            hstring_print(x);
            hstring_print(y);
            err = TRUE;
        }

        hstring_destroy(&x);
        hstring_destroy(&y);
    }
    measures_destroy (&hamming);
    //  @end

    printf(" OK\n");
}
/** @} */
