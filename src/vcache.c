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

struct _vcache_t {
    /* Cache structure */
    entry_t *cache;
    long space;
    long size;

    /* Cache statistics */
    double hits;
    double misses;

    /* Read-write cache for lock */
    rwlock_t rwlock;
};

/**
 * @defgroup vcache Value cache
 * Cache for similarity values based on uthash.
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */


//  --------------------------------------------------------------------------
//  Create new cache object

vcache_t *
vcache_new (config_t *cfg)
{
    assert (cfg);
    vcache_t *self = (vcache_t *) zmalloc (sizeof (vcache_t));

    //  Lookup cache size
    cfg_int csize;
    config_lookup_int (cfg, "measures.cache_size", &csize);

    //  Initialize cache stats
    self->space = floor ((csize * 1024 * 1024) / sizeof (entry_t));
    self->size = 0;
    self->misses = 0;
    self->hits = 0;

    info_msg (1, "Initializing cache with %dMb (%d entries)", csize, self->space);

    self->cache = (entry_t *) zmalloc (self->space * sizeof (entry_t));
    assert (self->cache);

    //  Initialize lock
    rwlock_init (&self->rwlock);

    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the value cache

void
vcache_destroy (vcache_t **self_p)
{
    assert (self_p);
    info_msg (1, "Clearing cache and freeing memory");
    if (*self_p) {
        vcache_t *self = *self_p;

        //  Destroy lock
        rwlock_destroy(&self->rwlock);

        //  Clear hash table
        free (self->cache);

        //  Free self
        free(self);
    }
}

//  --------------------------------------------------------------------------
//  Invalidate cache without freeing the memory

void
vcache_invalidate (vcache_t *self)
{
    assert (self);
    int index;
    for (index = 0; index < self->space; index++)
        self->cache[index].key = 0;
}


//  --------------------------------------------------------------------------
//  Store a similarity value. The value is associated with 64 bit key that
//  can be computed from a string, a sequence of symbols or even a pair
//  of strings. Collisions may occur, but are not likely.
//  @param key Key for similarity value
//  @param value Value to store
//  @param id ID of task
//  @return true on success, false otherwise

int
vcache_store (vcache_t *self, uint64_t key, float value, int id)
{
    int idx;

    idx = (key ^ id) % self->space;

    rwlock_set_wlock(&self->rwlock);

    if (self->cache[idx].key == 0)
        self->size++;

    self->cache[idx].key = key;
    self->cache[idx].val = value;
    self->cache[idx].id = id;
    rwlock_unset_wlock(&self->rwlock);

    return TRUE;
}


//  --------------------------------------------------------------------------
//  Load a similarity value. The value is associated with 64 bit key.
//  @param key Key for similarity value
//  @param value Pointer to space for value
//  @param id ID of task
//  @return true on success, false otherwise

int vcache_load(vcache_t *self, uint64_t key, float *value, int id)
{
    int ret, idx;

    idx = (key ^ id) % self->space;
    rwlock_set_rlock(&self->rwlock);
    if (self->cache[idx].key == key && self->cache[idx].id == id) {
        *value = self->cache[idx].val;
        ret = TRUE;
        self->hits++;
    } else {
        ret = FALSE;
        self->misses++;
    }

    rwlock_unset_rlock(&self->rwlock);
    return ret;
}


//  --------------------------------------------------------------------------
//  Display some information about cache usage

void vcache_info(vcache_t *self)
{
    float used = (self->size * sizeof(entry_t)) / (1024.0 * 1024.0);
    float free = (self->space * sizeof(entry_t)) / (1024.0 * 1024.0);

    info_msg(1,
             "Cache stats: %.1fMb used by %d entries, hits %3.0f%%, %.1fMb free.",
             used, self->size, 100 * self->hits / (self->hits + self->misses), free);
}


//  --------------------------------------------------------------------------
//  Get used memory in megabytes
//  @return used memory

float vcache_get_used(vcache_t *self)
{
    return (self->size * sizeof(entry_t)) / (1024.0 * 1024.0);
}


//  --------------------------------------------------------------------------
//  Get hit rate
//  @return hit rate

float vcache_get_hitrate(vcache_t *self)
{
    const double total = (self->hits + self->misses);
    return (total <= 0 ? 0 : 100 * self->hits / (self->hits + self->misses));
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
vcache_test (bool verbose)
{
    printf (" * vcache: SKIP.\n");
}
/** @} */
