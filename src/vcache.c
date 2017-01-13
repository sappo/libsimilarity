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

/* Cache structure */
static entry_t *cache = NULL;
static long space = 0;
static long size = 0;

/* Cache statistics */
static double hits = 0;
static double misses = 0;

/* Read-write cache for lock */
static rwlock_t rwlock;

/* Measures */
static long active_measures = 0;

/**
 * @defgroup vcache Value cache
 * Cache for similarity values based on uthash.
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

/**
 * Init value cache
 */
void vcache_init(config_t *cfg)
{
    if (active_measures == 0) {
        cfg_int csize;
        config_lookup_int(cfg, "measures.cache_size", &csize);

        /* Initialize cache stats */
        space = floor((csize * 1024 * 1024) / sizeof(entry_t));
        size = 0;
        misses = 0;
        hits = 0;

        info_msg(1, "Initializing cache with %dMb (%d entries)", csize, space);

        cache = (entry_t *) zmalloc(space * sizeof(entry_t));
        if (!cache)
            error("Failed to allocate value cache");

        /* Initialize lock */
        rwlock_init(&rwlock);
    }
    active_measures++;
}

/**
 * Store a similarity value. The value is associated with 64 bit key that
 * can be computed from a string, a sequence of symbols or even a pair
 * of strings. Collisions may occur, but are not likely.
 * @param key Key for similarity value
 * @param value Value to store
 * @param id ID of task
 * @return true on success, false otherwise
 */
int vcache_store(uint64_t key, float value, int id)
{
    int idx;

    idx = (key ^ id) % space;

    rwlock_set_wlock(&rwlock);

    if (cache[idx].key == 0)
        size++;

    cache[idx].key = key;
    cache[idx].val = value;
    cache[idx].id = id;
    rwlock_unset_wlock(&rwlock);

    return TRUE;
}

/**
 * Load a similarity value. The value is associated with 64 bit key.
 * @param key Key for similarity value
 * @param value Pointer to space for value
 * @param id ID of task
 * @return true on success, false otherwise
 */
int vcache_load(uint64_t key, float *value, int id)
{
    int ret, idx;

    idx = (key ^ id) % space;
    rwlock_set_rlock(&rwlock);
    if (cache[idx].key == key && cache[idx].id == id) {
        *value = cache[idx].val;
        ret = TRUE;
        hits++;
    } else {
        ret = FALSE;
        misses++;
    }

    rwlock_unset_rlock(&rwlock);
    return ret;
}

/**
 * Display some information about cache usage
 */
void vcache_info()
{
    float used = (size * sizeof(entry_t)) / (1024.0 * 1024.0);
    float free = (space * sizeof(entry_t)) / (1024.0 * 1024.0);

    info_msg(1,
             "Cache stats: %.1fMb used by %d entries, hits %3.0f%%, %.1fMb free.",
             used, size, 100 * hits / (hits + misses), free);
}

/**
 * Get used memory in megabytes
 * @return used memory
 */
float vcache_get_used()
{
    return (size * sizeof(entry_t)) / (1024.0 * 1024.0);
}

/**
 * Get hit rate
 * @return hit rate
 */
float vcache_get_hitrate()
{
    const double total = (hits + misses);
    return (total <= 0 ? 0 : 100 * hits / (hits + misses));
}

/**
 * Destroy the value cache
 */
void vcache_destroy()
{
    active_measures--;
    if (active_measures == 0) {
        info_msg(1, "Clearing cache and freeing memory");

        /* Destroy lock */
        rwlock_destroy(&rwlock);

        /* Clear hash table */
        free(cache);
    }
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
vcache_test (bool verbose)
{
    printf (" * vcache: SKIP.\n");
}
/** @} */
