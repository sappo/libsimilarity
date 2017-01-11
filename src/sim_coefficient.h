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

#ifndef SIM_COEFFICIENTS_H
#define SIM_COEFFICIENTS_H

typedef struct
{
    float a;    /**< Number of matching symbols */
    float b;    /**< Number of left mismatches */
    float c;    /**< Number of right mismatches */
} match_t;

void sim_coefficient_config(measures_t *self);
void sim_coefficient_apply_cfg(measures_t *self);

#define sim_jaccard_config sim_coefficient_config
float sim_jaccard_compare(measures_t *, hstring_t *x, hstring_t *y);

#define sim_simpson_config sim_coefficient_config
float sim_simpson_compare(measures_t *, hstring_t *x, hstring_t *y);

#define sim_braun_config sim_coefficient_config
float sim_braun_compare(measures_t *, hstring_t *x, hstring_t *y);

#define sim_dice_config sim_coefficient_config
float sim_dice_compare(measures_t *, hstring_t *x, hstring_t *y);

#define sim_sokal_config sim_coefficient_config
float sim_sokal_compare(measures_t *, hstring_t *x, hstring_t *y);

#define sim_kulczynski_config sim_coefficient_config
float sim_kulczynski_compare(measures_t *, hstring_t *x, hstring_t *y);

#define sim_otsuka_config sim_coefficient_config
float sim_otsuka_compare(measures_t *, hstring_t *x, hstring_t *y);

void sim_coefficient_test (bool verbose);
#endif /* SIM_COEFFICIENTS_H */
