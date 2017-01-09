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

/**
 * @defgroup default Configuration
 * Functions for configuration of the Harry tool. Additionally default
 * values for each configuration parameter are specified in this module.
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "harry_classes.h"

#define I "input"
#define M "measures"
#define O "output"

/* Default configuration */
static default_t defaults[] = {
    {I "", "input_format", CONFIG_TYPE_STRING, {.str = "lines"}},
    {I "", "chunk_size", CONFIG_TYPE_INT, {.num = 256}},
    {I "", "decode_str", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {I "", "fasta_regex", CONFIG_TYPE_STRING, {.str = " (\\+|-)?[0-9]+"}},
    {I "", "lines_regex", CONFIG_TYPE_STRING, {.str = "^(\\+|-)?[0-9]+"}},
    {I "", "reverse_str", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {I "", "stoptoken_file", CONFIG_TYPE_STRING, {.str = ""}},
    {I "", "soundex", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {M "", "measure", CONFIG_TYPE_STRING, {.str = "dist_levenshtein"}},
    {M "", "granularity", CONFIG_TYPE_STRING, {.str = "bytes"}},
    {M "", "token_delim", CONFIG_TYPE_STRING, {.str = " %0a%0d"}},
    {M "", "num_threads", CONFIG_TYPE_INT, {.num = 0}},
    {M "", "cache_size", CONFIG_TYPE_INT, {.num = 256}},
    {M "", "global_cache", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {M "", "col_range", CONFIG_TYPE_STRING, {.str = ""}},
    {M "", "row_range", CONFIG_TYPE_STRING, {.str = ""}},
    {M "", "split", CONFIG_TYPE_STRING, {.str = ""}},
    {M ".dist_hamming", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".dist_levenshtein", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".dist_levenshtein", "cost_ins", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_levenshtein", "cost_del", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_levenshtein", "cost_sub", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_damerau", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".dist_damerau", "cost_ins", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_damerau", "cost_del", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_damerau", "cost_sub", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_damerau", "cost_tra", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_osa", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".dist_osa", "cost_ins", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_osa", "cost_del", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_osa", "cost_sub", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_osa", "cost_tra", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".dist_jarowinkler", "scaling", CONFIG_TYPE_FLOAT, {.flt = 0.1}},
    {M ".dist_lee", "min_sym", CONFIG_TYPE_INT, {.num = 0}},
    {M ".dist_lee", "max_sym", CONFIG_TYPE_INT, {.num = 255}},
    {M ".dist_compression", "level", CONFIG_TYPE_INT, {.num = 9}},
    {M ".dist_bag", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".dist_kernel", "kern", CONFIG_TYPE_STRING, {.str = "kern_wdegree"}},
    {M ".dist_kernel", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".dist_kernel", "squared", CONFIG_TYPE_BOOL, {.num = CONFIG_TRUE}},
    {M ".kern_wdegree", "degree", CONFIG_TYPE_INT, {.num = 3}},
    {M ".kern_wdegree", "shift", CONFIG_TYPE_INT, {.num = 0}},
    {M ".kern_wdegree", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".kern_distance", "dist", CONFIG_TYPE_STRING, {.str = "dist_bag"}},
    {M ".kern_distance", "type", CONFIG_TYPE_STRING, {.str = "linear"}},
    {M ".kern_distance", "gamma", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".kern_distance", "degree", CONFIG_TYPE_FLOAT, {.flt = 1.0}},
    {M ".kern_distance", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".kern_subsequence", "length", CONFIG_TYPE_INT, {.num = 3}},
    {M ".kern_subsequence", "lambda", CONFIG_TYPE_FLOAT, {.flt = 0.1}},
    {M ".kern_subsequence", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".kern_spectrum", "length", CONFIG_TYPE_INT, {.num = 3}},
    {M ".kern_spectrum", "norm", CONFIG_TYPE_STRING, {.str = "none"}},
    {M ".sim_coefficient", "matching", CONFIG_TYPE_STRING, {.str = "bin"}},
    {O "", "output_format", CONFIG_TYPE_STRING, {.str = "text"}},
    {O "", "precision", CONFIG_TYPE_INT, {.num = 0}},
    {O "", "separator", CONFIG_TYPE_STRING, {.str = ","}},
    {O "", "save_indices", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {O "", "save_labels", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {O "", "save_sources", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {O "", "compress", CONFIG_TYPE_BOOL, {.num = CONFIG_FALSE}},
    {NULL}
};

/**
 * Print a configuration setting.
 * @param f File stream to print to
 * @param cs Configuration setting
 * @param d Current depth.
 */
static void config_setting_fprint(FILE *f, config_setting_t * cs, int d)
{
    assert(cs && d >= 0);

    int i;
    for (i = 0; i < d - 1; i++)
        fprintf(f, "       ");

    char *n = config_setting_name(cs);

    switch (config_setting_type(cs)) {
    case CONFIG_TYPE_GROUP:
        if (d > 0)
            fprintf(f, "%s = {\n", n);

        for (i = 0; i < config_setting_length(cs); i++)
            config_setting_fprint(f, config_setting_get_elem(cs, i), d + 1);

        if (d > 0) {
            for (i = 0; i < d - 1; i++)
                fprintf(f, "       ");
            fprintf(f, "};\n\n");
        }
        break;
    case CONFIG_TYPE_STRING:
        fprintf(f, "%s\t= \"%s\";\n", n, config_setting_get_string(cs));
        break;
    case CONFIG_TYPE_FLOAT:
        fprintf(f, "%s\t= %7.5f;\n", n, config_setting_get_float(cs));
        break;
    case CONFIG_TYPE_INT:
        fprintf(f, "%s\t= %ld;\n", n, (long) config_setting_get_int(cs));
        break;
    case CONFIG_TYPE_BOOL:
        fprintf(f, "%s\t= %s;\n", n, config_setting_get_bool(cs)
                ? "true" : "false");
        break;
    default:
        error("Unsupported type for configuration setting '%s'", n);
        break;
    }
}

/**
 * Print the configuration.
 * @param cfg configuration
 */
void config_print(config_t * cfg)
{
    config_setting_fprint(stdout, config_root_setting(cfg), 0);
}

/**
 * Print the configuration to a file.
 * @param f pointer to file stream
 * @param cfg configuration
 */
void config_fprint(FILE *f, config_t * cfg)
{
    config_setting_fprint(f, config_root_setting(cfg), 0);
}

/**
 * The functions add default values to unspecified parameters.
 * @param cfg configuration
 */
static void config_default(config_t * cfg)
{
    int i, b;
    cfg_int j;
    const char *s;
    double f;
    config_setting_t *cs = NULL, *vs;
    char *token, *string, *tofree;

    for (i = 0; defaults[i].name; i++) {
        /* Lookup and create setting group */
        tofree = string = strdup(defaults[i].group);
        vs = config_root_setting(cfg);
        while ((token = strsep(&string, ".")) != NULL) {
            cs = config_setting_get_member(vs, token);
            if (!cs)
                cs = config_setting_add(vs, token, CONFIG_TYPE_GROUP);
            vs = cs;
        }
        free(tofree);

        switch (defaults[i].type) {
        case CONFIG_TYPE_STRING:
            if (config_setting_lookup_string(cs, defaults[i].name, &s))
                continue;

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_STRING);
            config_setting_set_string(vs, defaults[i].val.str);
            break;
        case CONFIG_TYPE_FLOAT:
            if (config_setting_lookup_float(cs, defaults[i].name, &f))
                continue;

            /* Check for mis-interpreted integer */
            if (config_setting_lookup_int(cs, defaults[i].name, &j)) {
                config_setting_remove(cs, defaults[i].name);
                vs = config_setting_add(cs, defaults[i].name,
                                        CONFIG_TYPE_FLOAT);
                config_setting_set_float(vs, (double) j);
                continue;
            }

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_FLOAT);
            config_setting_set_float(vs, defaults[i].val.flt);
            break;
        case CONFIG_TYPE_INT:
            if (config_setting_lookup_int(cs, defaults[i].name, &j))
                continue;

            /* Check for mis-interpreted float */
            if (config_setting_lookup_float(cs, defaults[i].name, &f)) {
                config_setting_remove(cs, defaults[i].name);
                vs = config_setting_add(cs, defaults[i].name,
                                        CONFIG_TYPE_INT);
                config_setting_set_int(vs, (long) round(f));
                continue;
            }

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_INT);
            config_setting_set_int(vs, defaults[i].val.num);
            break;
        case CONFIG_TYPE_BOOL:
            if (config_setting_lookup_bool(cs, defaults[i].name, &b))
                continue;

            /* Check for mis-interpreted integer */
            if (config_setting_lookup_int(cs, defaults[i].name, &j)) {
                config_setting_remove(cs, defaults[i].name);
                vs = config_setting_add(cs, defaults[i].name,
                                        CONFIG_TYPE_BOOL);
                config_setting_set_bool(vs,
                                        j == 0 ? CONFIG_FALSE : CONFIG_TRUE);
                continue;
            }

            /* Add default value */
            config_setting_remove(cs, defaults[i].name);
            vs = config_setting_add(cs, defaults[i].name, CONFIG_TYPE_BOOL);
            config_setting_set_bool(vs, defaults[i].val.num);
            break;

        }
    }
}

/**
 * Checks if the configuration is valid and sane.
 * @return 1 if config is valid, 0 otherwise
 */
int config_check(config_t * cfg)
{
    const char *str1, *str2;

    /* Add default values where missing */
    config_default(cfg);

    /* Sanity checks for tokens */
    config_lookup_string(cfg, "measures.granularity", &str1);
    config_lookup_string(cfg, "measures.token_delim", &str2);

    if (!strcasecmp(str1, "tokens") && strlen(str2) == 0) {
        error("Delimiters are required if the granularity is tokens.");
        return 0;
    }

    return 1;
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
hconfig_test (bool verbose)
{
    printf (" * hconfig: SKIP.\n");
}

/** @} */
