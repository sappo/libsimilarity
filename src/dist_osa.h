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

#ifndef DIST_OSA_H
#define DIST_OSA_H

/* Module interface */
void dist_osa_config(measures_t *self);
float dist_osa_compare(measures_t *, hstring_t *, hstring_t *);
void dist_osa_test (bool verbose);

#endif /* DIST_OSA_H */
