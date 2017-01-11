/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2013-2014 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details.
 */

/**
 * @defgroup measures Measure interface
 *
 * Interface and functions for computing similarity measures for strings
 *
 * @{
 */

#include "harry_classes.h"

//  TODO: remove when ported all methods
int verbose;
int log_line;
config_t *cfg;

/* Module interfaces */
measures_func_t func[] = {
    {"dist_bag", dist_bag_config, dist_bag_compare},
    {"dist_compression", dist_compression_config, dist_compression_compare},
    {"dist_ncd", dist_compression_config, dist_compression_compare},
    {"dist_damerau", dist_damerau_config, dist_damerau_compare},
    {"dist_hamming", dist_hamming_config, dist_hamming_compare},
    {"dist_jaro", dist_jaro_config, dist_jaro_compare},
    {"dist_jarowinkler", dist_jarowinkler_config, dist_jarowinkler_compare},
    {"dist_kernel", dist_kernel_config, dist_kernel_compare},
    {"dist_lee", dist_lee_config, dist_lee_compare},
    {"dist_levenshtein", dist_levenshtein_config, dist_levenshtein_compare},
    {"dist_edit", dist_levenshtein_config, dist_levenshtein_compare},
    {"dist_osa", dist_osa_config, dist_osa_compare},
    {"kern_distance", kern_distance_config, kern_distance_compare},
    {"kern_dsk", kern_distance_config, kern_distance_compare},
    {"kern_spectrum", kern_spectrum_config, kern_spectrum_compare},
    {"kern_ngram", kern_spectrum_config, kern_spectrum_compare},
    {"kern_subsequence", kern_subsequence_config, kern_subsequence_compare},
    {"kern_ssk", kern_subsequence_config, kern_subsequence_compare},
    {"kern_wdegree", kern_wdegree_config, kern_wdegree_compare},
    {"kern_wdk", kern_wdegree_config, kern_wdegree_compare},
    {"sim_braun", sim_braun_config, sim_braun_compare},
    {"sim_dice", sim_dice_config, sim_dice_compare},
    {"sim_czekanowski", sim_dice_config, sim_dice_compare},
    {"sim_jaccard", sim_jaccard_config, sim_jaccard_compare},
    {"sim_kulczynski", sim_kulczynski_config, sim_kulczynski_compare},
    {"sim_otsuka", sim_otsuka_config, sim_otsuka_compare},
    {"sim_ochiai", sim_otsuka_config, sim_otsuka_compare},
    {"sim_simpson", sim_simpson_config, sim_simpson_compare},
    {"sim_sokal", sim_sokal_config, sim_sokal_compare},
    {"sim_anderberg", sim_sokal_config, sim_sokal_compare},
    {NULL}
};


//  --------------------------------------------------------------------------
//  Creates a new measures instance for the given measure function. Return a
//  measures instance initialized with measure function default values or NULL
//  if measure function could not be found.

measures_t *
measures_new (const char *name)
{
    assert (name);
    if (measures_match (name) == -1)
        return NULL;

    int rc = 0;
    measures_t *self = (measures_t *) zmalloc (sizeof (measures_t));
    self->opts = (measures_opts_t *) zmalloc (sizeof (measures_opts_t));
    // Init configuration
    self->cfg = (config_t *) zmalloc (sizeof (config_t));
    config_init (self->cfg);
    assert (self->cfg);
    rc = config_check (self->cfg);
    assert (rc != 0);

    /* Init value cache */
    vcache_init(self->cfg);

    measures_config (self, name);
    self->idx = 0;
    self->verbose = 0;
    self->log_line = 0;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy a measures instance.

void
measures_destroy (measures_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        measures_t *self = *self_p;
        config_destroy(self->cfg);
        vcache_destroy();
        free (self->cfg);
        free (self->opts);
        free (self);
    }
}


/**
 * Find best matching measure name.
 * @param name to check
 */
int
measures_match(const char *name)
{
    int r = 0;
    float d, dist = FLT_MAX;

    for (int i = 0; func[i].name; i++) {
        /* Match full name */
        d = fabs(strcasecmp(name, func[i].name));
        if (d < dist) {
            r = i, dist = d;
        }

        /* Match from second token */
        char *token = strchr(func[i].name, '_') + 1;
        d = fabs(strcasecmp(name, token));
        if (d < dist) {
            r = i, dist = d;
        }
    }

    if (dist > 0) {
        warning("Unknown measure '%s'. Using '%s' instead.", name,
                func[r].name);
        return -1;
    }

    return r;
}

/**
 * Configures the measure for a given similarity measure.
 * @param name Name of similarity measure
 * @return name of selected similarity measure
 */
char *
measures_config (measures_t *self, const char *name)
{
    assert (self);
    assert (name);
    const char *cfg_str;

    /* Set delimiters */
    config_lookup_string(self->cfg, "measures.token_delim", &cfg_str);
    if (strlen(cfg_str) > 0)
        hstring_delim_set(cfg_str);
    else
        hstring_delim_reset();

    /* Enable global cache */
    config_lookup_int(self->cfg, "measures.global_cache", &self->global_cache);

    /* Configure */
    self->idx = measures_match(name);
    self->func = &func[self->idx];
    self->func->measure_config(self);
    return self->func->name;
}

/**
 * Print list of supported similarity measures
 * @param f File stream
 */
void
measures_fprint (FILE *f)
{
        fprintf(f,
           "    dist_bag             Bag distance\n"
           "    dist_compression     Normalized compression distance (NCD)\n"
           "    dist_damerau         Damerau-Levenshtein distance\n"
           "    dist_hamming         Hamming distance\n"
           "    dist_jaro            Jaro distance\n"
           "    dist_jarowinkler     Jaro-Winkler distance\n"
           "    dist_kernel          Kernel substitution distance\n"
           "    dist_lee             Lee distance\n"
           "    dist_levenshtein     Levenshtein distance\n"
           "    dist_osa             Optimal string alignment (OSA) distance\n"
           "    kern_distance        Distance substitution kernel (DSK)\n"
           "    kern_spectrum        Spectrum kernel\n"
           "    kern_subsequence     Subsequence kernel (SSK)\n"
           "    kern_wdegree         Weighted-degree kernel (WDK)\n"
           "    sim_braun            Braun-Blanquet coefficient\n"
           "    sim_dice             Soerensen-Dice coefficient\n"
           "    sim_jaccard          Jaccard coefficient\n"
           "    sim_kulczynski       second Kulczynski coefficient\n"
           "    sim_otsuka           Otsuka coefficient\n"
           "    sim_simpson          Simpson coefficient\n"
           "    sim_sokal            Sokal-Sneath coefficient\n"
    );

}

/**
 * Compares two strings with the given similarity measure.
 * @param x first string
 * @param y second second
 * @return similarity/dissimilarity value
 */
float
measures_compare(measures_t *self, hstring_t *x, hstring_t *y)
{
    if (!self->global_cache)
        return self->func->measure_compare(self, x, y);

    uint64_t xyk = hstring_hash2(x, y);
    float m = 0;

    if (!vcache_load(xyk, &m, ID_COMPARE)) {
        m = self->func->measure_compare(self, x, y);
        vcache_store(xyk, m, ID_COMPARE);
    }
    return m;
}


//  --------------------------------------------------------------------------
//  Sets a string configuration

void
measures_config_set_string (measures_t *self, const char *key, const char *value)
{
    assert (self);
    assert (key);
    assert (value);
    config_set_string (self->cfg, key, value);
    self->func->measure_config (self);
}


//  --------------------------------------------------------------------------
//  Sets a integer configuration

void
measures_config_set_int (measures_t *self, const char *key, const int value)
{
    assert (self);
    assert (key);
    config_set_int (self->cfg, key, value);
    self->func->measure_config (self);
}


//  --------------------------------------------------------------------------
//  Sets a float configuration

void
measures_config_set_float (measures_t *self, const char *key, const float value)
{
    assert (self);
    assert (key);
    config_set_float (self->cfg, key, value);
    self->func->measure_config (self);
}


//  --------------------------------------------------------------------------
//  Sets a boolean configuration

void
measures_config_set_bool (measures_t *self, const char *key, const bool value)
{
    assert (self);
    assert (key);
    config_set_bool (self->cfg, key, value);
    self->func->measure_config (self);
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
measures_test (bool verbose)
{
    printf (" * measures: OK.\n");
}

/** @} */
