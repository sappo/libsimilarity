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
 * <em>dist_kernel</em>: Kernel-based distance
 *
 * This module implements a kernel-based distance. A given kernel function
 * is mapped to a Euclidean distance using simple geometry.
 * @{
 */

/* External variables */
extern config_t cfg;
extern measures_func_t func[];

/* Normalizations */
static knorm_t norm = KN_NONE;
static int kern = 0;
static int squared = 1;

/**
 * Initializes the similarity measure
 */
void dist_kernel_config(measures_t *self)
{
    const char *str;

    /* Kernel measure */
    config_lookup_string(self->cfg, "measures.dist_kernel.kern", &str);
    kern = measures_match(str);
    func[kern].measure_config(self);

    /* Parameters */
    config_lookup_bool(self->cfg, "measures.dist_kernel.squared", &squared);

    /* Normalization */
    config_lookup_string(self->cfg, "measures.dist_kernel.norm", &str);
    norm = knorm_get(str);
}

/**
 * Internal kernel function.
 * @param x first string
 * @param y second string
 * @return kernel value
 */
static float kernel(measures_t *self, hstring_t *x, hstring_t *y)
{
    double k = func[kern].measure_compare(self, x, y);
    return knorm(self, k, x, y, func[kern].measure_compare);
}

/**
 * Compute a kernel-based distance
 * @param x first string
 * @param y second string
 * @return kernel-based distance
 */
float dist_kernel_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    assert (self);
    float k1, k2, k3;
    uint64_t xk, yk;
    float d = 0;

    xk = hstring_hash1(x);
    if (!vcache_load(xk, &k1, ID_DIST_KERNEL)) {
        k1 = kernel(self, x, x);
        vcache_store(xk, k1, ID_DIST_KERNEL);
    }

    yk = hstring_hash1(y);
    if (!vcache_load(yk, &k2, ID_DIST_KERNEL)) {
        k2 = kernel(self, y, y);
        vcache_store(yk, k2, ID_DIST_KERNEL);
    }

    /* Not cached here */
    k3 = kernel(self, x, y);
    d = k1 + k2 - 2 * k3;

    return squared ? d : sqrt(d);
}

/** @} */
