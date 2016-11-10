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

/**
 * @defgroup string String object
 * Functions for processing strings and sequences
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "harry_classes.h"

/* Global delimiter table */
char delim[256] = { DELIM_NOT_INIT };

/**
 * Structure for stop tokens (irrelevant tokens)
 */
typedef struct
{
    sym_t sym;                  /* Hash of stop token */
    UT_hash_handle hh;          /* uthash handle */
} stoptoken_t;
static stoptoken_t *stoptokens = NULL;

//  --------------------------------------------------------------------------
//  Converts a c-style string into a string object.

hstring_t *
hstring_new (const char *str)
{
    assert (str);
    hstring_t *self = (hstring_t *) zmalloc (sizeof (hstring_t));
    self->str.c = strdup (str);
    self->type = TYPE_BYTE;
    self->len = strlen(self->str.c);
    self->src = NULL;

    return self;
}

/**
 * Free memory of the string object
 * @param x string object
 */
void hstring_destroy(hstring_t *x)
{
    switch (x->type) {
    case TYPE_BYTE:
    case TYPE_BIT:
        if (x->str.c)
            free(x->str.c);
        break;
    case TYPE_TOKEN:
        if (x->str.s)
            free(x->str.s);
        break;
    }

    if (x->src)
        free(x->src);

    /* Make sure everything is null */
    x->str.c = NULL;
    x->str.s = NULL;
    x->src = NULL;
    x->len = 0;
}


/**
 * Convert a c-style string to a string object. New memory is allocated
 * and the string is copied.
 * @param x string object
 * @param s c-style string
 */
hstring_t hstring_init(hstring_t x, char *s)
{
    x.str.c = strdup(s);
    x.type = TYPE_BYTE;
    x.len = strlen(s);
    x.src = NULL;

    return x;
}


/**
 * Check whether delimiters have been set
 * @return true if delimiters have been set
 */
int hstring_has_delim()
{
    return (delim[0] != DELIM_NOT_INIT);
}

/**
 * Return symbol/character at given positions
 * @param x string x
 * @param i position in string x
 * @return character/symbol
 */
sym_t hstring_get(hstring_t x, int i)
{
    assert(i < x.len);
    int b;

    switch (x.type) {
    case TYPE_TOKEN:
        return x.str.s[i];
    case TYPE_BYTE:
        return x.str.c[i];
    case TYPE_BIT:
        b = x.str.c[i / 8];
        return b >> (7 - i % 8) & 1;
    default:
        error("Unknown string type");
        return 0;
    }
}


/**
 * Print string object
 * @param x string object
 */
void hstring_print(hstring_t x)
{
    int i;

    if (x.type == TYPE_BIT && x.str.c) {
        for (i = 0; i < x.len; i++)
            printf("%d", (int) hstring_get(x, i));
        printf(" (bits)\n");
    }

    if (x.type == TYPE_BYTE && x.str.c) {
        for (i = 0; i < x.len; i++)
            if (isprint(x.str.c[i]))
                printf("%c", x.str.c[i]);
            else
                printf("%%%.2x", (char) x.str.c[i]);
        printf(" (bytes)\n");
    }

    if (x.type == TYPE_TOKEN && x.str.s) {
        for (i = 0; i < x.len; i++)
            printf("%" PRIu64 " ", (uint64_t) x.str.s[i]);
        printf(" (tokens)\n");
    }

    printf("  [type: %d, len: %d; src: %s, label: %f]\n",
           x.type, x.len, x.src, x.label);
}

/**
 * Decodes a string containing delimiters to a lookup table
 * @param s String containing delimiters
 */
void hstring_delim_set(const char *s)
{
    char buf[5] = "0x00";
    unsigned int i, j;

    if (strlen(s) == 0) {
        hstring_delim_reset();
        return;
    }

    memset(delim, 0, 256);
    for (i = 0; i < strlen(s); i++) {
        if (s[i] != '%') {
            delim[(unsigned int) s[i]] = 1;
            continue;
        }

        /* Skip truncated sequence */
        if (strlen(s) - i < 2)
            break;

        buf[2] = s[++i];
        buf[3] = s[++i];
        sscanf(buf, "%x", (unsigned int *) &j);
        delim[j] = 1;
    }
}

/**
 * Resets delimiters table. There is a global table of delimiter
 * symbols which is only initialized once the first sequence is
 * processed. This functions is used to trigger a re-initialization.
 */
void hstring_delim_reset()
{
    delim[0] = DELIM_NOT_INIT;
}



/**
 * Converts a string into a sequence of tokens using delimiter characters.
 * The original character string is lost.
 * @param x character string
 * @return string of tokens
 */
hstring_t hstring_tokenify(hstring_t x)
{
    int i = 0, j = 0, k = 0, dlm = 0;
    int wstart = 0;


    /* A string of n chars can have at most n/2 + 1 tokens */
    sym_t *sym = (sym_t *) zmalloc((x.len / 2 + 1) * sizeof(sym_t));
    if (!sym) {
        error("Failed to allocate memory for symbols");
        return x;
    }

    /* Find first delimiter symbol */
    for (dlm = 0; !delim[(unsigned char) dlm] && dlm < 256; dlm++);

    /* Remove redundant delimiters */
    for (i = 0, j = 0; i < x.len; i++) {
        if (delim[(unsigned char) x.str.c[i]]) {
            if (j == 0 || delim[(unsigned char) x.str.c[j - 1]])
                continue;
            x.str.c[j++] = (char) dlm;
        } else {
            x.str.c[j++] = x.str.c[i];
        }
    }

    /* Extract tokens */
    for (wstart = i = 0; i < j + 1; i++) {
        /* Check for delimiters and remember start position */
        if ((i == j || x.str.c[i] == dlm) && i - wstart > 0) {
            /* Hash token */
            uint64_t hash = hash_str(x.str.c + wstart, i - wstart);
            sym[k++] = (sym_t) hash;
            wstart = i + 1;
        }
    }
    x.len = k;
    sym = (sym_t *) realloc(sym, x.len * sizeof(sym_t));

    /* Change representation */
    free(x.str.c);
    x.str.s = sym;
    x.type = TYPE_TOKEN;

    return x;
}

/**
 * Converts a string into a sequence of bits. Well, actually there is no
 * conversion except for that the counting now happens on the level of bits
 * instead of bytes.
 * @param x character string
 * @return string of bits
 */
hstring_t hstring_bitify(hstring_t x)
{
    x.len = x.len * 8;
    x.type = TYPE_BIT;
    return x;
}


/**
 * Create an empty string
 * @param x string object
 * @param t granularity of string
 */
hstring_t hstring_empty(hstring_t x, int t)
{
    x.str.c = (char *) zmalloc(0);
    x.type = t;
    x.label = 1.0;
    x.len = 0;
    x.src = NULL;

    return x;
}

/**
 * Compute a 64-bit hash for a string. The hash is used at different locations.
 * Collisions are possible but not very likely (hopefully)
 * @param x String to hash
 * @return hash value
 */
uint64_t hstring_hash1(hstring_t x)
{
    if (x.type == TYPE_BIT && x.str.c)
        return MurmurHash64B(x.str.c, sizeof(char) * x.len / 8, 0xc0ffee);
    if (x.type == TYPE_BYTE && x.str.c)
        return MurmurHash64B(x.str.c, sizeof(char) * x.len, 0xc0ffee);
    if (x.type == TYPE_TOKEN && x.str.s)
        return MurmurHash64B(x.str.s, sizeof(sym_t) * x.len, 0xc0ffee);

    warning("Nothing to hash. String is missing");
    return 0;
}

/**
 * Compute a 64-bit hash for a substring.
 * Collisions are possible but not very likely (hopefully)
 * @param x String to hash
 * @param i Start of substring
 * @param l Length of substring
 * @return hash value
 */
uint64_t hstring_hash_sub(hstring_t x, int i, int l)
{

    if (i > x.len - 1 || i + l > x.len) {
        warning("Invalid range for substring (i:%d;l:%d;x:%d)", i, l, x.len);
        return 0;
    }

    if (x.type == TYPE_BIT && x.str.c) {
        error("Substrings are currently not supported for bits");
        return 0;
    }

    if (x.type == TYPE_BYTE && x.str.c)
        return MurmurHash64B(x.str.c + i, sizeof(char) * l, 0xc0ffee);
    if (x.type == TYPE_TOKEN && x.str.s)
        return MurmurHash64B(x.str.s + i, sizeof(sym_t) * l, 0xc0ffee);

    warning("Nothing to hash. String is missing");
    return 0;
}



/*+
 * Swap the high and low 32 bits of a 64 bit integer
 * @param x integer
 * @return integer with swapped bits
 */
static uint64_t swap(uint64_t x)
{
    uint64_t r = 0;
    r |= x << 32;
    r |= x >> 32;
    return r;
}

/**
 * Compute a 64-bit hash for two strings. The computation is symmetric, that is,
 * the same strings retrieve the same hash independent of their order.
 * Collisions are possible but not very likely (hopefully)
 * @param x String to hash
 * @param y String to hash
 * @return hash value
 */
uint64_t hstring_hash2(hstring_t x, hstring_t y)
{
    uint64_t a, b;

    if (x.type == TYPE_BIT && y.type == TYPE_BIT && x.str.c && y.str.c) {
        a = MurmurHash64B(x.str.c, sizeof(char) * x.len / 8, 0xc0ffee);
        b = MurmurHash64B(y.str.c, sizeof(char) * y.len / 8, 0xc0ffee);
        return swap(a) ^ b;
    }
    if (x.type == TYPE_BYTE && y.type == TYPE_BYTE && x.str.c && y.str.c) {
        a = MurmurHash64B(x.str.c, sizeof(char) * x.len, 0xc0ffee);
        b = MurmurHash64B(y.str.c, sizeof(char) * y.len, 0xc0ffee);
        return swap(a) ^ b;
    }
    if (x.type == TYPE_TOKEN && y.type == TYPE_TOKEN && x.str.s && y.str.s) {
        a = MurmurHash64B(x.str.s, sizeof(sym_t) * x.len, 0xc0ffee);
        b = MurmurHash64B(y.str.s, sizeof(sym_t) * y.len, 0xc0ffee);
        return swap(a) ^ b;
    }

    warning("Nothing to hash. Strings are missing or incompatible.");
    return 0;
}


/**
 * Read in and hash stop tokens
 * @param file stop token file
 */
void stoptokens_load(const char *file)
{
    char buf[1024];
    FILE *f;

    info_msg(1, "Loading stop tokens from '%s'.", file);
    if (!(f = fopen(file, "r")))
        fatal("Could not read stop token file %s", file);

    /* Read stop tokens */
    while (fgets(buf, 1024, f)) {
        int len = strip_newline(buf, strlen(buf));
        if (len <= 0)
            continue;

        /* Decode URI-encoding */
        decode_str(buf);

        /* Add stop token to hash table */
        stoptoken_t *token = (stoptoken_t *) zmalloc(sizeof(stoptoken_t));
        token->sym = (sym_t) hash_str(buf, len);
        HASH_ADD(hh, stoptokens, sym, sizeof(sym_t), token);
    }
    fclose(f);
}

/**
 * Filter stop tokens from symbols
 * @param x Symbolized string
 */
hstring_t stoptokens_filter(hstring_t x)
{
    assert(x.type == TYPE_TOKEN);
    stoptoken_t *stoptoken;
    int i, j;

    for (i = j = 0; i < x.len; i++) {
        /* Check for stop token */
        sym_t sym = x.str.s[i];
        HASH_FIND(hh, stoptokens, &sym, sizeof(sym_t), stoptoken);

        /* Remove stoptoken */
        if (stoptoken)
            continue;

        if (i != j)
            x.str.s[j] = x.str.s[i];
        j++;
    }
    x.len = j;
    return x;
}

/**
 * Preprocess a given string
 * @param x character string
 * @return preprocessed string
 */
hstring_t hstring_preproc(measures_t *measure, hstring_t x)
{
    assert(x.type == TYPE_BYTE);
    int decode, reverse, soundex, c, i, k;
    const char *gran;

    config_lookup_string(measure->cfg, "measures.granularity", &gran);
    config_lookup_bool(measure->cfg, "input.decode_str", &decode);
    config_lookup_bool(measure->cfg, "input.reverse_str", &reverse);
    config_lookup_bool(measure->cfg, "input.soundex", &soundex);

    if (decode) {
        x.len = decode_str(x.str.c);
        x.str.c = (char *) realloc(x.str.c, x.len);
    }

    if (reverse) {
        for (i = 0, k = x.len - 1; i < k; i++, k--) {
            c = x.str.c[i];
            x.str.c[i] = x.str.c[k];
            x.str.c[k] = c;
        }
    }

    if (soundex)
        x = hstring_soundex(x);

    if (!strcasecmp(gran, "bytes")) {
        /* nothing */
    } else if (!strcasecmp(gran, "tokens")) {
        assert(hstring_has_delim());
        x = hstring_tokenify(x);
    } else if (!strcasecmp(gran, "bits")) {
        x = hstring_bitify(x);
    } else {
        error("Unknown granularity '%s'. Using 'bytes' instead.", gran);
    }

    if (stoptokens)
        x = stoptokens_filter(x);

    return x;
}

/**
 * Destroy stop tokens table
 */
void stoptokens_destroy()
{
    stoptoken_t *s;

    while (stoptokens) {
        s = stoptokens;
        HASH_DEL(stoptokens, s);
        free(s);
    }
}

/**
 * Soundex code as implemented by Kevin Setter, 8/27/97 with some
 * slight modifications. Known bugs: Consonants separated by a vowel
 * are treated as one character, if they have the same index. This
 * is wrong. :(
 *
 * @param in input string
 * @param len end of input string
 * @param out output buffer of 5 bytes
 * @return soundex
 */
static void soundex(char *in, int len, char *out)
{
    int i = 0, j = 0;
    char c, prev = '*';

    /* Skip first letter if in the following set */
    switch (tolower(in[0])) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'y':
    case 'h':
    case 'w':
        i++;
        j++;
        break;
    }

    while (i < len && j <= 4) {
        in[i] = tolower(in[i]);
        switch (in[i]) {
        case 'b':
            c = '1';
            break;
        case 'p':
            c = '1';
            break;
        case 'f':
            c = '1';
            break;
        case 'v':
            c = '1';
            break;
        case 'c':
            c = '2';
            break;
        case 's':
            c = '2';
            break;
        case 'k':
            c = '2';
            break;
        case 'g':
            c = '2';
            break;
        case 'j':
            c = '2';
            break;
        case 'q':
            c = '2';
            break;
        case 'x':
            c = '2';
            break;
        case 'z':
            c = '2';
            break;
        case 'd':
            c = '3';
            break;
        case 't':
            c = '3';
            break;
        case 'l':
            c = '4';
            break;
        case 'm':
            c = '5';
            break;
        case 'n':
            c = '5';
            break;
        case 'r':
            c = '6';
            break;
        default:
            c = '*';
        }
        if ((c != prev) && (c != '*')) {
            out[j] = c;
            prev = out[j];
            j++;
        }
        i++;
    }

    if (j < 4)
        for (i = j; i < 4; i++)
            out[i] = '0';


    out[0] = toupper(in[0]);
    out[4] = 0;
}



/**
 * Perform a soundex transformation of each token.
 * @param x string
 */
hstring_t hstring_soundex(hstring_t x)
{
    int start = 0, i, alloc = 0, end = 0;
    char sdx[5], *out = NULL;

    assert(x.type == TYPE_BYTE);

    for (i = 0; i < x.len; i++) {
        /* Compute soundex for each substring of letters */
        if (i == x.len - 1 || !isalpha(x.str.c[i])) {
            if (start < i) {
                int len = i - start + ((i == x.len - 1) ? 1 : 0);
                soundex(x.str.c + start, len, sdx);
            }
            start = i + 1;

            /* Reallocate memory if necessary */
            if (end + 6 > alloc) {
                alloc += 128;   /* Use small blocks */
                out = (char *) realloc(out, alloc);
            }

            /* Append soundex */
            snprintf(out + end, 6, "%s ", sdx);
            end += 5;
        }
    }

    /* Overwrite original stirng data */
    free(x.str.c);
    x.str.c = out;
    x.len = end - 1;
    return x;
}


//  --------------------------------------------------------------------------
//  Self test of this class


void
hstring_test (bool verbose)
{
    printf (" * hstring: ");
    printf ("OK\n");
}

/** @} */
