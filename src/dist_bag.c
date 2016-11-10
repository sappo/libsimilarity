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
 * @{
 */

/* Hash table */
typedef struct
{
    sym_t sym;          /**< Symbol or character */
    float cnt;          /**< Count of symbol */
    UT_hash_handle hh;  /**< uthash handle */
} bag_t;

/* Static variables */
static lnorm_t n = LN_NONE;

/* External variables */
extern config_t cfg;

/**
 * Initializes the similarity measure
 */
void dist_bag_config()
{
    const char *str;

    /* Normalization */
    config_lookup_string(&cfg, "measures.dist_bag.norm", &str);
    n = lnorm_get(str);
}

/**
 * Computes a histogram of symbols or characters
 * @param x string
 * @return histogram
 */
static bag_t *bag_create(hstring_t *x)
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
float dist_bag_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    float xd = 0, yd = 0;
    bag_t *xh, *yh, *xb, *yb;

    xh = bag_create(x);
    yh = bag_create(y);

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

    bag_destroy(xh);
    bag_destroy(yh);

    return lnorm(n, fmax(xd, yd), x, y);
}

/** @} */
