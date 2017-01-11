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
 * <em>kern_wdegree</em>: Weighted-degree kernel
 *
 * Sonnenburg, Raetsch, and Rieck. Large scale learning with string
 * kernels. In Large Scale Kernel Machines, pages 73--103. MIT Press,
 * 2007.
 * @{
 */

/**
 * Initializes the similarity measure
 */
void kern_wdegree_config(measures_t *self)
{
    assert (self);
    measures_opts_t* opts = self->opts;
    const char *str;

    config_lookup_int(self->cfg, "measures.kern_wdegree.degree", &opts->degree);
    config_lookup_int(self->cfg, "measures.kern_wdegree.shift", &opts->shift);

    /* Normalization */
    config_lookup_string(self->cfg, "measures.kern_wdegree.norm", &str);
    opts->knorm = knorm_get(str);
}

/**
 * Weighting function for matching blocks.
 * Sonnenburg, Raetsch, Schoelkopf: Large scale genomic sequence SVM
 * classifiers.  ICML 2005: 848-855
 * @param len length of block
 * @param degree of kernel
 * @return weighting
 */
static float weight(float len, int degree)
{
    assert(len > 0 && degree > 0);

    if (len <= degree) {
        float w = len * (-len * len + 3 * degree * len + 3 * degree + 1);
        return w / (3 * degree * (degree + 1));
    } else {
        return (3 * len - degree + 1) / 3;
    }
}

/**
 * Implementation of weighted-degree kernel in block mode.
 * @param x String x
 * @param y String y
 * @param xs Shift for x
 * @param ys Shift for y
 * @param len Length of region to match
 * @return kernel value
 */
static float kern_wdegree(measures_t *self, hstring_t *x, hstring_t *y, int xs, int ys, int len)
{
    measures_opts_t *opts = self->opts;
    int i, start;
    float k = 0;

    for (i = 0, start = -1; i < len; i++) {
        /* Identify matching region */
        if (!hstring_compare(x, i + xs, y, i + ys)) {
            if (start == -1)
                start = i;
            continue;
        }

        /* No match found continue */
        if (start == -1)
            continue;

        k += weight(i - start, opts->degree);
        start = -1;
    }

    if (start != -1)
        k += weight(i - start, opts->degree);

    return k;
}

/**
 * Internal computation of weighted-degree kernel with shift
 * @param x first string
 * @param y second string
 * @return weighted-degree kernel
 */
static float kernel(measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    float k = 0;
    int s, len;

    /* Loop over shifts */
    for (s = -opts->shift; s <= opts->shift; s++) {
        if (s <= 0) {
            len = fmax(fmin(x->len, y->len + s), 0);
            k += kern_wdegree(self, x, y, 0, -s, len);
        } else {
            len = fmax(fmin(x->len - s, y->len), 0);
            k += kern_wdegree(self, x, y, +s, 0, len);
        }
    }

    return k;
}


/**
 * Compute the weighted-degree kernel with shift. If the strings have
 * unequal size, the remaining symbols of the longer string are ignored (in
 * accordance with the kernel definition)
 * @param x first string
 * @param y second string
 * @return weighted-degree kernel
 */
float kern_wdegree_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    assert (self);
    float k = kernel(self, x, y);
    return knorm(self, k, x, y, kernel);
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
kern_wdegree_test (bool verbose)
{
    printf (" * kern_wdegree: SKIP.\n");
}
/** @} */
