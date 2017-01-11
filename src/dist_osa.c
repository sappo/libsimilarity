/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2014 Konrad Rieck (konrad@mlsec.org)
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
 * <em>dist_osa</em>: Optimal sequence alignment (OSA) distance
 *
 * Doolittle. Of Urfs and Orfs: A Primer on How to Analyze Derived Amino
 * Acid Sequences. University Science Books, 1986
 @{
 */

/**
 * Initializes the similarity measure
 */
void dist_osa_config(measures_t *self)
{
    assert(self);
    measures_opts_t *opts = self->opts;
    const char *str;

    /* Costs */
    config_lookup_float(self->cfg, "measures.dist_osa.cost_ins", &opts->cost_ins);
    config_lookup_float(self->cfg, "measures.dist_osa.cost_del", &opts->cost_del);
    config_lookup_float(self->cfg, "measures.dist_osa.cost_sub", &opts->cost_sub);
    config_lookup_float(self->cfg, "measures.dist_osa.cost_tra", &opts->cost_tra);

    /* Normalization */
    config_lookup_string(self->cfg, "measures.dist_osa.norm", &str);
    opts->lnorm = lnorm_get(str);
}

/* Ugly macros to access arrays */
#define D(i,j) 		d[(i) * (y->len + 1) + (j)]

/**
 * Computes the OSA distance of two strings.
 * @param x first string
 * @param y second string
 * @return OSA distance
 */
float dist_osa_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    int i, j, a, b, c;

    if (x->len == 0 && y->len == 0)
        return 0;

    /* Allocate matrix. We might reduce this to some rows only */
    int *d = (int *) calloc((x->len + 1) * (y->len + 1), sizeof(int));

    /* Init margin of matrix */
    for (i = 0; i <= x->len; i++)
        D(i, 0) = i * opts->cost_ins;
    for (j = 0; j <= y->len; j++)
        D(0, j) = j * opts->cost_ins;

    for (i = 1; i <= x->len; i++) {
        for (j = 1; j <= y->len; j++) {

            /* Comparison */
            c = hstring_compare(x, i - 1, y, j - 1);

            /* Insertion an deletion */
            a = D(i - 1, j) + opts->cost_ins;
            b = D(i, j - 1) + opts->cost_del;
            if (a > b)
                a = b;

            /* Substitution */
            b = D(i - 1, j - 1) + (c ? opts->cost_sub : 0);
            if (a > b)
                a = b;

            /* Transposition */
            if (i > 1 && j > 1 &&
                !hstring_compare(x, i - 1, y, j - 2) &&
                !hstring_compare(x, i - 2, y, j - 1)) {
                b = D(i - 2, j - 2) + (c ? opts->cost_tra : 0);
                if (a > b)
                    a = b;
            }

            /* Update matrix */
            D(i, j) = a;
        }
    }

    double m = D(x->len, y->len);
    free(d);

    return lnorm(opts->lnorm, m, x, y);
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
    /* Comparison using bytes */
    {"", "", 0},
    {"a", "", 1},
    {"", "a", 1},
    {"a", "a", 0},
    {"ca", "abc", 3},
    {NULL}
};

void
dist_osa_test (bool verbose)
{
    printf(" * OSA distance:");

    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *osa = measures_new ("dist_osa");
    assert (osa);

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        hstring_preproc (x, osa);
        hstring_preproc (y, osa);

        float d = measures_compare (osa, x, y);
        double diff = fabs (tests[i].v - d);

        if (diff > 1e-6) {
            printf("Error %f != %f\n", d, tests[i].v);
            hstring_print(x);
            hstring_print(y);
            assert (false);
        }

        hstring_destroy(&x);
        hstring_destroy(&y);
    }
    measures_destroy (&osa);
    //  @end

    printf(" OK\n");
}
/** @} */
