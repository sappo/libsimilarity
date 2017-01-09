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

/**
 * @defgroup rwlock Read-write lock
 *
 * Wrapper for read-write lock. If the POSIX thread library is available,
 * the corresponding lock is directly used.  Otherwise the implementation
 * resorts to an inefficient OpenMP mutex.  :(
 *
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

/**
 * Init a read-write lock.
 * @param rw pointer to lock object
 */
void rwlock_init(rwlock_t *rw)
{
#ifdef ENABLE_PRWLOCK
    pthread_rwlock_init(&rw->lock, NULL);
#else
#ifdef HAVE_OPENMP
    omp_init_lock(&rw->lock);
#endif
#endif
}

/**
 * Destroy a read-write lock.
 * @param rw pointer to lock object
 */
void rwlock_destroy(rwlock_t *rw)
{
#ifdef ENABLE_PRWLOCK
    pthread_rwlock_destroy(&rw->lock);
#else
#ifdef HAVE_OPENMP
    omp_destroy_lock(&rw->lock);
#endif
#endif
}

/**
 * Set lock for reading.
 * @param rw pointer to lock object
 */
void rwlock_set_rlock(rwlock_t *rw)
{
#ifdef ENABLE_PRWLOCK
    pthread_rwlock_rdlock(&rw->lock);
#else
#ifdef HAVE_OPENMP
    omp_set_lock(&rw->lock);
#endif
#endif
}

/**
 * Unset lock for reading.
 * @param rw pointer to lock object
 */
void rwlock_unset_rlock(rwlock_t *rw)
{
#ifdef ENABLE_PRWLOCK
    pthread_rwlock_unlock(&rw->lock);
#else
#ifdef HAVE_OPENMP
    omp_unset_lock(&rw->lock);
#endif
#endif
}

/**
 * Set lock for writing.
 * @param rw pointer to lock object
 */
void rwlock_set_wlock(rwlock_t *rw)
{
#ifdef ENABLE_PRWLOCK
    pthread_rwlock_wrlock(&rw->lock);
#else
#ifdef HAVE_OPENMP
    omp_set_lock(&rw->lock);
#endif
#endif
}

/**
 * Unset lock for writing.
 * @param rw pointer to lock object
 */
void rwlock_unset_wlock(rwlock_t *rw)
{
#ifdef ENABLE_PRWLOCK
    pthread_rwlock_unlock(&rw->lock);
#else
#ifdef HAVE_OPENMP
    omp_unset_lock(&rw->lock);
#endif
#endif
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
rwlock_test (bool verbose)
{
    printf (" * rwlock: SKIP.\n");
}
/** @} */
