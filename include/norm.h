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

#ifndef NORM_H
#define NORM_H

/* Length normalizations */
typedef enum
{
    LN_NONE,
    LN_MIN,
    LN_MAX,
    LN_AVG,
} lnorm_t;

lnorm_t lnorm_get(const char *str);
float lnorm(lnorm_t n, float d, hstring_t *x, hstring_t *y);

/* Kernel normalizations */
typedef enum
{
    KN_NONE,
    KN_L2
} knorm_t;

knorm_t knorm_get(const char *str);
float knorm(measures_t *self, float k, hstring_t *x, hstring_t *y,
            float (*kernel) (measures_t *, hstring_t *, hstring_t *));

void
    norm_test (bool verbose);

#endif /* NORM_H */
