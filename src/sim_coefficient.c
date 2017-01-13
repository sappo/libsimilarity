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
 * <em>sim_simpson</em>: Simpson jaccard <br/>
 * <em>sim_jaccard</em>: Jaccard jaccard <br/>
 * <em>sim_braun</em>: Braun-Blanquet jaccard <br/>
 * <em>sim_dice</em>: Dice jaccard (Czekanowsi, Soerensen-Dice) <br/>
 * <em>sim_sokal</em>: Sokal-Sneath jaccard (Anderberg) <br/>
 * <em>sim_kulczynski</em>: second Kulczynski jaccard <br/>
 * <em>sim_otsuka</em>: Otsuka jaccard (Ochiai) <br/>
 * @{
 */

typedef struct
{
    sym_t sym;          /**< Symbol or character */
    float cnt;          /**< Count of symbol */
    UT_hash_handle hh;  /**< uthash handle */
} bag_t;


void sim_coefficient_config(measures_t *self)
{
    assert (self);
    measures_opts_t *opts = self->opts;
    const char *str;

    /* Matching */
    config_lookup_string (self->cfg, "measures.sim_coefficient.matching", &str);

    if (!strcasecmp(str, "cnt")) {
        opts->binary = FALSE;
    } else if (!strcasecmp(str, "bin")) {
        opts->binary = TRUE;
    } else {
        warning("Unknown matching '%s'. Using 'cnt' instead.", str);
        opts->binary = FALSE;
    }
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
 * Computes the matches and mismatches
 * @param x first string
 * @param y second string
 * @return matches
 */
static match_t match(measures_t *self, hstring_t *x, hstring_t *y)
{
    measures_opts_t *opts = self->opts;
    bag_t *xh, *yh, *xb, *yb;
    match_t m;
    int missing;

    m.a = 0;
    m.b = 0;
    m.c = 0;

    xh = bag_create(x);
    yh = bag_create(y);

    if (!opts->binary) {
        /* Count matching */
        missing = y->len;
        for (xb = xh; xb != NULL; xb = (bag_t *) xb->hh.next) {
            HASH_FIND(hh, yh, &(xb->sym), sizeof(sym_t), yb);
            if (!yb) {
                m.b += xb->cnt;
            } else {
                m.a += fmin(xb->cnt, yb->cnt);
                missing -= fmin(xb->cnt, yb->cnt);
                if (yb->cnt < xb->cnt)
                    m.b += xb->cnt - yb->cnt;
            }
        }
        m.c += missing;
    } else {
        /* Binary matching */
        missing = HASH_COUNT(yh);
        for (xb = xh; xb != NULL; xb = (bag_t *) xb->hh.next) {
            HASH_FIND(hh, yh, &(xb->sym), sizeof(sym_t), yb);
            if (!yb) {
                m.b += 1;
            } else {
                m.a += 1;
                missing -= 1;
            }
        }
        m.c += missing;
    }

    bag_destroy(xh);
    bag_destroy(yh);
    return m;
}

/**
 * Computes the Jaccard jaccard
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_jaccard_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return m.a / (m.a + m.b + m.c);
}

/**
 * Computes the Simpson jaccard
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_simpson_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return m.a / fmin(m.a + m.b, m.a + m.c);
}

/**
 * Computes the Braun-Blanquet jaccard
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_braun_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return m.a / fmax(m.a + m.b, m.a + m.c);
}

/**
 * Computes the Dice efficient
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_dice_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return 2 * m.a / (2 * m.a + m.b + m.c);
}

/**
 * Computes the Sokal-Sneath efficient
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_sokal_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return m.a / (m.a + 2 * (m.b + m.c));
}

/**
 * Computes the Kulczynski (2nd) efficient
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_kulczynski_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return 0.5 * (m.a / (m.a + m.b) + m.a / (m.a + m.c));
}

/**
 * Computes the Otsuka efficient
 * @param x String x
 * @param y String y
 * @return jaccard
 */
float sim_otsuka_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    match_t m = match(self, x, y);
    if (m.b == 0 && m.c == 0)
        return 1;

    return m.a / sqrt((m.a + m.b) * (m.a + m.c));
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
    char *m;            /**< Mode */
    float v;            /**< Expected output */
};

static struct hstring_test tests[] = {
    /* Jaccard jaccard 1 */
    {"", "", "bin", 1.0},
    {"a", "", "bin", 0.0},
    {"", "a", "bin", 0.0},
    {"ab", "ab", "bin", 1.0},
    {"ba", "ab", "bin", 1.0},
    {"bbcc", "bbbd", "bin", 1.0 / (1.0 + 2.0)},
    {"bbcc", "bbbd", "cnt", 2.0 / (2.0 + 4.0)},
    {"bbcc", "bbbdc", "bin", 2.0 / (2.0 + 1.0)},
    {"bbbdc", "bbcc", "bin", 2.0 / (2.0 + 1.0)},
    {"bbbdc", "bbcc", "cnt", 3.0 / (3.0 + 3.0)},
    {"bbcc", "bbbyc", "cnt", 3.0 / (3.0 + 3.0)},
    {NULL}
};


static struct hstring_test tests_dice[] = {
    /* Dice Simmetrics */
    {NULL}
};


void
sim_coefficient_test (bool verbose)
{
    printf (" * Sim Coefficient:");
    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *jaccard = measures_new ("sim_jaccard");
    assert (jaccard);

    for (i = 0; tests[i].x && !err; i++) {
        measures_config_set_string (jaccard , "measures.sim_coefficient.matching", tests[i].m);
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        hstring_preproc (x, jaccard);
        hstring_preproc (y, jaccard);

        float d = measures_compare (jaccard, x, y);
        double diff = fabs (tests[i].v - d);

        if (diff > 1e-6) {
            printf("Error %f != %f\n", d, tests[i].v);
            hstring_print(x);
            hstring_print(y);
            assert (false);
        }

        hstring_destroy (&x);
        hstring_destroy (&y);
    }
    measures_destroy (&jaccard);


    measures_t *dice = measures_new ("sim_dice");
    assert (dice);

    for (i = 0; tests_dice[i].x && !err; i++) {
        measures_config_set_string (dice , "measures.sim_coefficient.matching", tests_dice[i].m);
        x = hstring_new (tests_dice[i].x);
        y = hstring_new (tests_dice[i].y);

        hstring_preproc (x, dice);
        hstring_preproc (y, dice);

        float d = measures_compare (dice, x, y);
        double diff = fabs (tests_dice[i].v - d);

        if (diff > 1e-6) {
            printf("Error %f != %f\n", d, tests_dice[i].v);
            hstring_print(x);
            hstring_print(y);
            assert (false);
        }

        hstring_destroy (&x);
        hstring_destroy (&y);
    }
    measures_destroy (&dice);
    //  @end

    printf(" OK\n");
}
/** @} */
