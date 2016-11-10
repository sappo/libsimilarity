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
 * <em>kern_distance</em>: Distance substitution kernel
 *
 * This module implements distance substitution kernels by
 * Haasdonk and Bahlmann (2004).  The empty string is considered as
 * the origin of the underlying implicit vector space.
 * @{
 */

/* External variables */
extern measure_func_t func[];

/* Normalizations */
static knorm_t norm = KN_NONE;
static subst_t subst = DS_LINEAR;
static int dist = 0;
static double fgamma = 1.0;
static double degree = 1.0;

/**
 * Initializes the similarity measure
 */
void kern_distance_config(measures_t *self)
{
    const char *str;

    /* Distance measure */
    config_lookup_string(self->cfg, "measures.kern_distance.dist", &str);
    dist = measure_match(str);
    func[dist].measure_config(self);

    /* Substitution type */
    config_lookup_string(self->cfg, "measures.kern_distance.type", &str);
    if (!strcasecmp(str, "linear")) {
        subst = DS_LINEAR;
    } else if (!strcasecmp(str, "poly")) {
        subst = DS_POLY;
    } else if (!strcasecmp(str, "neg")) {
        subst = DS_NEG;
    } else if (!strcasecmp(str, "rbf")) {
        subst = DS_RBF;
    } else {
        warning("Unknown substitution type '%s'. Using 'linear'.", str);
        subst = DS_LINEAR;
    }

    /* Parameters */
    config_lookup_float(self->cfg, "measures.kern_distance.fgamma", &fgamma);
    config_lookup_float(self->cfg, "measures.kern_distance.degree", &degree);

    /* Normalization */
    config_lookup_string(self->cfg, "measures.kern_distance.norm", &str);
    norm = knorm_get(str);

}

/**
 * Internal computation of an inner product in an implicit feature space.
 * The inner-product space is created by centering the distances at the
 * empty string. Note that if the distance is not Euclidean, the computed
 * result is undefined and a feature space may not exist.
 *
 * @param x String x
 * @param y String y
 * @return inner product (dot)
 */
static float dot(measures_t *self, hstring_t x, hstring_t y)
{
    hstring_t o;
    uint64_t xk, yk;
    float d1, d2, d3;

    o = hstring_empty(o, x.type);

    xk = hstring_hash1(x);
    if (!vcache_load(xk, &d1, ID_KERN_DISTANCE)) {
        d1 = func[dist].measure_compare(self, x, o);
        vcache_store(xk, d1, ID_KERN_DISTANCE);
    }

    yk = hstring_hash1(y);
    if (!vcache_load(yk, &d2, ID_KERN_DISTANCE)) {
        d2 = func[dist].measure_compare(self, y, o);
        vcache_store(yk, d2, ID_KERN_DISTANCE);
    }

    /* Not cached here */
    d3 = func[dist].measure_compare(self, x, y);
    return -0.5 * (d3 * d3 - d2 * d2 - d1 * d1);
}

/**
 * Internal computation of distance substitution kernel
 * @param x first string
 * @param y second string
 * @return distance substitution kernel
 */
static float kernel(measures_t *self, hstring_t x, hstring_t y)
{
    float d, k = 0;

    switch (subst) {
    default:
    case DS_LINEAR:
        k = dot(self, x, y);
        break;
    case DS_POLY:
        k = pow(1 + fgamma * dot(self, x, y), degree);
        break;
    case DS_NEG:
        d = func[dist].measure_compare(self, x, y);
        k = -pow(d, degree);
        break;
    case DS_RBF:
        d = func[dist].measure_compare(self, x, y);
        k = exp(-fgamma * d * d);
        break;
    }

    return k;
}

/**
 * Compute a distance substitution kernel
 * @param x first string
 * @param y second string
 * @return distance substitution kernel
 */
float kern_distance_compare(measures_t *self, hstring_t x, hstring_t y)
{
    assert (self);
    float k = kernel(self, x, y);
    return knorm(self, k, x, y, kernel);
}

/** @} */
