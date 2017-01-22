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

#ifndef VCACHE_H
#define VCACHE_H

/** Task identifiers */
#define ID_COMPARE              1       /* Global comparison cache */
#define ID_DIST_COMPRESS	2       /* Compression distance */
#define ID_NORM                 3       /* Normalization */
#define ID_KERN_DISTANCE	4       /* Distance substitution kernel */
#define ID_DIST_KERNEL		5       /* Kernel-based distance */

typedef struct
{
    uint64_t key;       /**< Hash for sequences */
    int id;             /**< ID of task */
    float val;          /**< Cached similarity value */
} entry_t;

vcache_t *
vcache_new (config_t *cfg);
void
vcache_destroy (vcache_t **self_p);
void
vcache_invalidate (vcache_t *self);
int vcache_load (vcache_t *self, uint64_t key, float *value, int);
int vcache_store (vcache_t *self, uint64_t key, float value, int);
void vcache_info (vcache_t *self);
float vcache_get_hitrate (vcache_t *self);
float vcache_get_used (vcache_t *self);
void vcache_test (bool verbose);

#endif
