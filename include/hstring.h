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

#ifndef HSTRING_H
#define HSTRING_H

/** Placeholder for non-initialized delimiters */
#define DELIM_NOT_INIT  42

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

/* Support types for strings. See union str in struct */
#define TYPE_BYTE		0x00
#define TYPE_TOKEN		0x01
#define TYPE_BIT		0x02

//  Converts a c-style string into a string object.

hstring_t *
    hstring_new (const char *str);

void hstring_print(hstring_t);
void hstring_delim_set(const char *);
void hstring_delim_reset();
hstring_t hstring_tokenify(hstring_t);
hstring_t hstring_bitify(hstring_t);
hstring_t hstring_preproc(measures_t *measure, hstring_t x);
hstring_t hstring_empty(hstring_t, int type);
hstring_t hstring_init(hstring_t, char *);
void hstring_destroy(hstring_t *);
uint64_t hstring_hash_sub(hstring_t x, int i, int l);
uint64_t hstring_hash1(hstring_t);
uint64_t hstring_hash2(hstring_t, hstring_t);
int hstring_has_delim();
sym_t hstring_get(hstring_t x, int i);
hstring_t hstring_soundex(hstring_t);

/* Additional functions */
void stoptokens_load(const char *f);
void stoptokens_destroy();

/* Inline functions */

/**
 * Compare two symbols/characters
 * @param x string x
 * @param i position in string x
 * @param y string y
 * @param j position in string y
 * @return 0 if equal, < 0 if x smaller, > 0 if y smaller
 */
static inline int hstring_compare(hstring_t x, int i, hstring_t y, int j)
{
    assert(x.type == y.type);
    assert(i < x.len && j < y.len);
    int a, b;

    switch (x.type) {
    case TYPE_BIT:
        a = x.str.c[i / 8] >> (7 - i % 8) & 1;
        b = y.str.c[j / 8] >> (7 - j % 8) & 1;
        return (a - b);
    case TYPE_TOKEN:
        return (x.str.s[i] - y.str.s[j]);
    case TYPE_BYTE:
        return (x.str.c[i] - y.str.c[j]);
    default:
        error("Unknown string type");
    }
    return 0;
}

void
    hstring_test (bool verbose);

#endif /* HSTRING_H */
