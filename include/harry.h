/*  =========================================================================
    harry - Harry - A Tool for Measuring String Similarity

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.  This program is distributed without any
    warranty. See the GNU General Public License for more details.
    =========================================================================
*/

#ifndef HARRY_H_H_INCLUDED
#define HARRY_H_H_INCLUDED

//  Add your own public definitions here, if you need them
#define config_set_string(c,x,s) \
      config_setting_set_string(config_lookup(c,x),s)
#define config_set_int(c,x,s) \
      config_setting_set_int(config_lookup(c,x),s)
#define config_set_float(c,x,s) \
      config_setting_set_float(config_lookup(c,x),s)
#define config_set_bool(c,x,s) \
      config_setting_set_bool(config_lookup(c,x),s)

typedef int cfg_int;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//  Include the project library file
#include "harry_library.h"

#ifdef HAVE_UTHASH_UTHASH_H
#include <uthash/uthash.h>
#else
#ifdef HAVE_UTHASH_H
#include <uthash.h>
#else
#include "uthash.h"
#endif
#endif

/*
 * Symbols for tokens. Note: Some measures enumerate all possible symbols.
 * These need to be patched first to support larger symbol sizes.
 */
typedef uint64_t sym_t;

/**
 * Structure for a string
 */
struct _hstring_t
{
    union
    {
        char *c;              /**< Byte or bit representation */
        sym_t *s;             /**< Word representation */
    } str;

    int len;                  /**< Length of string */
    unsigned int type;        /**< Type of string */

    char *src;                /**< Optional source of string */
    float label;              /**< Optional label of string */
};

#endif
