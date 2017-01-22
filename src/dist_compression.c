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
 * <em>dist_compression</em>: Compression distance for strings.
 *
 * Cilibrasi and Vitanyi. Clustering by compression, IEEE Transactions on
 * Information Theory, 51:4, 1523-1545, 2005.
 * @{
 */

/**
 * Initializes the similarity measure
 */
void
dist_compression_config (measures_t *self)
{
    assert(self);
    measures_opts_t *opts = self->opts;
    /* Configuration */
    config_lookup_int (self->cfg, "measures.dist_compression.level", &opts->level);
}


/**
 * Compress one string and return the length of the compressed data
 * @param x String x
 * @return length of the compressed data
 */
static float
compress_str1 (measures_t *self, hstring_t *x)
{
    unsigned long tmp, width;
    unsigned char *dst;

    width = x->type == HSTRING_TYPE_TOKEN ? sizeof(sym_t) : sizeof(char);
    tmp = compressBound(x->len * width);

    dst = (unsigned char *) zmalloc(tmp);
    if (!dst) {
        error("Failed to allocate memory for compression");
        return -1;
    }

    compress2(dst, &tmp, (const Bytef *) x->str.c, x->len * width, self->opts->level);

    free(dst);
    return (float) tmp;
}

/**
 * Compress two strings and return the length of the compressed data
 * @param x String x
 * @param y String y
 * @return length of the compressed data.
 */
static float
compress_str2 (measures_t *self, hstring_t *x, hstring_t *y)
{
    unsigned long tmp, width;
    unsigned char *src, *dst;

    assert(x->type == y->type);

    width = x->type == HSTRING_TYPE_TOKEN ? sizeof(sym_t) : sizeof(char);
    tmp = compressBound((x->len + y->len) * width);

    dst = (unsigned char *) zmalloc(tmp);
    src = (unsigned char *) zmalloc(tmp);
    if (!src || !dst) {
        error("Failed to allocate memory for compression");
        return -1;
    }

    /* Concatenate sequences y and x */
    memcpy(src, y->str.s, y->len * width);
    memcpy(src + y->len * width, x->str.s, x->len * width);

    compress2(dst, &tmp, src, (x->len + y->len) * width, self->opts->level);

    free(dst);
    free(src);
    return (float) tmp;
}



/**
 * Computes the compression distance of two strings.
 * @param x first string
 * @param y second string
 * @return Compression distance
 */
float dist_compression_compare (measures_t *self, hstring_t *x, hstring_t *y)
{
    float xl, yl, xyl, yxl;
    uint64_t xk, yk, xyk, yxk;

    xk = hstring_hash1(x);
    if (!vcache_load(self->cache, xk, &xl, ID_DIST_COMPRESS)) {
        xl = compress_str1(self, x);
        vcache_store(self->cache, xk, xl, ID_DIST_COMPRESS);
    }

    yk = hstring_hash1(y);
    if (!vcache_load(self->cache, yk, &yl, ID_DIST_COMPRESS)) {
        yl = compress_str1(self, y);
        vcache_store(self->cache, yk, yl, ID_DIST_COMPRESS);
    }

    xyk = hstring_hash2(x, y);
    if (!vcache_load(self->cache, xyk, &xyl, ID_DIST_COMPRESS)) {
        xyl = compress_str2(self, x, y);
        vcache_store(self->cache, xyk, xyl, ID_DIST_COMPRESS);
    }

    yxk = hstring_hash2(y, x);
    if (!vcache_load(self->cache, yxk, &yxl, ID_DIST_COMPRESS)) {
        yxl = compress_str2(self, y, x);
        vcache_store(self->cache, yxk, yxl, ID_DIST_COMPRESS);
    }

    /* Symmetric version of distance */
    return (0.5 * (xyl + yxl) - fmin(xl, yl)) / fmax(xl, yl);
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
    {"", "abc", 0.272727},
    {"abc", "", 0.272727},
    {"abc", "abc", 0.272727},
    {"dslgjasldjfkasdjlkf", "dslkfjasldkf", 0.518519},
    {"kasjhdgkjad", "kasjhdgkjad", 0.105263},
    {"fkjhskljfhalsdkfhalksjdfhsdf", "djfh", 0.727273},
    {"fkjhskljfhalsdkfhalksjdfhsdf", "", 0.757576},
    {"", "fkjhskljfhalsdkfhalksjdfhsdf", 0.757576},
    {"6s6sd7as6d", "7sad8asd76", 0.444444},
    {"aaaaaaaaaa", "bbbbbbbbb", 0.272727},
    {NULL}
};

void
dist_compression_test (bool verbose)
{
    printf(" * Compression distance:");

    //  @selftest
    int i, err = FALSE;
    hstring_t *x, *y;
    measures_t *compression = measures_new ("dist_compression");
    assert (compression);

    for (i = 0; tests[i].x && !err; i++) {
        x = hstring_new (tests[i].x);
        y = hstring_new (tests[i].y);

        hstring_preproc (x, compression);
        hstring_preproc (y, compression);

        float d = measures_compare (compression, x, y);
        double diff = fabs (tests[i].v - d);

        if (diff > 1e-6) {
            printf("Error %f != %f\n", d, tests[i].v);
            hstring_print (x);
            hstring_print (y);
            err = TRUE;
        }

        hstring_destroy (&x);
        hstring_destroy (&y);
    }
    measures_destroy (&compression);
    //  @end
    //
    printf(" OK.\n");
}
