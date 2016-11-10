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

#ifndef KERN_SPECTRUM_H
#define KERN_SPECTRUM_H

/* Module interface */
void kern_spectrum_config();
float kern_spectrum_compare(measures_t *, hstring_t, hstring_t);

#endif /* KERN_SPECTRUM_H */
