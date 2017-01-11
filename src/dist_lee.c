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
 * <em>dist_lee</em>: Lee distance for strings.
 *
 * Lee. Some properties of nonbinary error-correcting codes. IRE
 * Transactions on Information Theory 4 (2): 77-82, 1958.
 * @{
 */

/**
 * Initializes the similarity measure
 */
void dist_lee_config(measures_t *self)
{
    assert(self);
    measures_opts_t *opts = self->opts;

    config_lookup_int(self->cfg, "measures.dist_lee.min_sym", &opts->min_sym);
    config_lookup_int(self->cfg, "measures.dist_lee.max_sym", &opts->max_sym);
}

/**
 * Computes the Lee distance of two strings. If the strings have
 * different lengths, the remaining symbols of the longer string are
 * added to the distance.
 * @param x first string
 * @param y second string
 * @return Lee distance
 */
float dist_lee_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    float d = 0, ad;
    int i, q = opts->max_sym - opts->min_sym;

    /* Loop over strings */
    for (i = 0; i < x->len || i < y->len; i++) {
        if (i < x->len && i < y->len)
            ad = fabs(hstring_compare(x, i, y, i) - opts->min_sym);
        else if (i < x->len)
            ad = fabs(hstring_get(x, i) - opts->min_sym);
        else
            ad = fabs(hstring_get(y, i) - opts->min_sym);

        if (ad > q) {
            warning("Distance of symbols larger than alphabet. Fixing.");
            ad = q - 1;
        }
        d += fmin(ad, q - ad);
    }

    return d;
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
    float v;            /**< Expected output */
};

static struct hstring_test tests[] = {
    /* comparison using bytes */
    {"", "", 0},
    {"a", "", 97},
    {"", "a", 97},
    {"a", "a", 0},
    {"ab", "ba", 2},
    {"bab", "ba", 98},
    {"\xff", "", 1},
    {"\x01", "", 1},
    {NULL}
};

void
dist_lee_test (bool verbose)
{
    printf (" * Lee distance:");

    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *lee = measures_new ("dist_lee");
    assert (lee);

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        hstring_preproc (x, lee);
        hstring_preproc (y, lee);

        float d = measures_compare (lee, x, y);
        double diff = fabs(tests[i].v - d);

        if (diff > 1e-6) {
            printf("Error %f != %f\n", d, tests[i].v);
            hstring_print (x);
            hstring_print (y);
            assert (false);
        }

        hstring_destroy (&x);
        hstring_destroy (&y);
    }
    measures_destroy (&lee);
    //  @end

    printf(" OK\n");
}
/** @} */
