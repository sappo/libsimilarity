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

#ifndef MEASURES_H
#define MEASURES_H

//  Init function
typedef void (measure_config_fn) (
    measures_t *);
//  Comparison function
typedef float (measure_compare_fn) (
    measures_t *, hstring_t *, hstring_t *);

/**
 * Structure for measure interface
 */
typedef struct
{
    char *name;     // Name of measure
    measure_config_fn *measure_config;      // Init function
    measure_compare_fn *measure_compare;    // Comparison function
} measure_func_t;

typedef struct
{
    lnorm_t lnorm;
    knorm_t knorm;
    double cost_ins;
    double cost_del;
    double cost_sub;
    double cost_tra;
} measures_opts_t;

struct _measures_t {
    config_t *cfg;
    measure_func_t *func;
    measures_opts_t *opts;
    cfg_int global_cache;
    int idx;
    int verbose;
    int log_line;
};

/* Module functions */

measures_t *
    measure_new (const char *name);

void
    measure_destroy (measures_t **self_p);

int
    measure_match(const char *);

char *
    measure_config (measures_t *self, const char *name);

double
    measure_compare(measures_t *self, hstring_t *x, hstring_t *y);

void
    measure_fprint(FILE *);

void
    measures_test (bool verbose);

#endif /* MEASURES_H */
