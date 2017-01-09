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

#ifndef DIST_HAMMING_H
#define DIST_HAMMING_H

/* Module interface */
void dist_hamming_config();
float dist_hamming_compare(measures_t *, hstring_t *, hstring_t *);
void dist_hamming_test (bool verbose);

#endif /* DIST_HAMMING_H */
