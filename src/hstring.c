/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2013-2015 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty-> See the GNU General Public License for more details.
 */

/**
 * @defgroup string String object
 * Functions for processing strings and sequences
 * @author Konrad Rieck (konrad@mlsec.org)
 * @{
 */

#include "harry_classes.h"

/* Global delimiter table */
char delim[256] = { HSTRING_DELIM_NOT_INIT };

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
    self->type = HSTRING_TYPE_BYTE;
    self->len = strlen(self->str.c);
    self->src = NULL;

    return self;
}

/**
 * Free memory of the string object
 * @param x string object
 */
void
hstring_destroy (hstring_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hstring_t *self = *self_p;

        switch (self->type) {
        case HSTRING_TYPE_BYTE:
        case HSTRING_TYPE_BIT:
            if (self->str.c)
                free(self->str.c);
            break;
        case HSTRING_TYPE_TOKEN:
            if (self->str.s)
                free(self->str.s);
            break;
        }

        if (self->src)
            free(self->src);

        /* Make sure everything is null */
        self->str.c = NULL;
        self->str.s = NULL;
        self->src = NULL;
        self->len = 0;
        free (self);
    }
}


/**
 * Check whether delimiters have been set
 * @return true if delimiters have been set
 */
int hstring_has_delim()
{
    return (delim[0] != HSTRING_DELIM_NOT_INIT);
}

/**
 * Return symbol/character at given positions
 * @param x string x
 * @param i position in string x
 * @return character/symbol
 */
sym_t
hstring_get (hstring_t *self, int i)
{
    assert(i < self->len);
    int b;

    switch (self->type) {
    case HSTRING_TYPE_TOKEN:
        return self->str.s[i];
    case HSTRING_TYPE_BYTE:
        return self->str.c[i];
    case HSTRING_TYPE_BIT:
        b = self->str.c[i / 8];
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
void
hstring_print (hstring_t *self)
{
    int i;

    if (self->type == HSTRING_TYPE_BIT && self->str.c) {
        for (i = 0; i < self->len; i++)
            printf ("%d", (int) hstring_get(self, i));
        printf(" (bits)\n");
    }

    if (self->type == HSTRING_TYPE_BYTE && self->str.c) {
        for (i = 0; i < self->len; i++)
            if (isprint(self->str.c[i]))
                printf("%c", self->str.c[i]);
            else
                printf("%%%.2x", (char) self->str.c[i]);
        printf(" (bytes)\n");
    }

    if (self->type == HSTRING_TYPE_TOKEN && self->str.s) {
        for (i = 0; i < self->len; i++)
            printf("%" PRIu64 " ", (uint64_t) self->str.s[i]);
        printf(" (tokens)\n");
    }

    printf("  [type: %d, len: %d; src: %s, label: %f]\n",
           self->type, self->len, self->src, self->label);
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
    delim[0] = HSTRING_DELIM_NOT_INIT;
}



/**
 * Converts a string into a sequence of tokens using delimiter characters.
 * The original character string is lost.
 * @param x character string
 * @return 0 if successful, otherwise -1
 */
int
hstring_tokenify (hstring_t *self)
{
    int i = 0, j = 0, k = 0, dlm = 0;
    int wstart = 0;


    /* A string of n chars can have at most n/2 + 1 tokens */
    sym_t *sym = (sym_t *) zmalloc((self->len / 2 + 1) * sizeof(sym_t));
    if (!sym) {
        error("Failed to allocate memory for symbols");
        return -1;
    }

    /* Find first delimiter symbol */
    for (dlm = 0; !delim[(unsigned char) dlm] && dlm < 256; dlm++);

    /* Remove redundant delimiters */
    for (i = 0, j = 0; i < self->len; i++) {
        if (delim[(unsigned char) self->str.c[i]]) {
            if (j == 0 || delim[(unsigned char) self->str.c[j - 1]])
                continue;
            self->str.c[j++] = (char) dlm;
        } else {
            self->str.c[j++] = self->str.c[i];
        }
    }

    /* Extract tokens */
    for (wstart = i = 0; i < j + 1; i++) {
        /* Check for delimiters and remember start position */
        if ((i == j || self->str.c[i] == dlm) && i - wstart > 0) {
            /* Hash token */
            uint64_t hash = hash_str(self->str.c + wstart, i - wstart);
            sym[k++] = (sym_t) hash;
            wstart = i + 1;
        }
    }
    self->len = k;
    sym = (sym_t *) realloc(sym, self->len * sizeof(sym_t));

    /* Change representation */
    free(self->str.c);
    self->str.s = sym;
    self->type = HSTRING_TYPE_TOKEN;
    return 0;
}

/**
 * Converts a string into a sequence of bits. Well, actually there is no
 * conversion except for that the counting now happens on the level of bits
 * instead of bytes.
 * @param x character string
 * @return string of bits
 */
void
hstring_bitify (hstring_t *self)
{
    self->len = self->len * 8;
    self->type = HSTRING_TYPE_BIT;
}


/**
 * Create an empty string
 * @param x string object
 * @param t granularity of string
 */
hstring_t *
hstring_empty (int t)
{
    hstring_t *self = (hstring_t *) zmalloc (sizeof (hstring_t));
    self->str.c = (char *) zmalloc(0);
    self->type = t;
    self->label = 1.0;
    self->len = 0;
    self->src = NULL;

    return self;
}

/**
 * Compute a 64-bit hash for a string. The hash is used at different locations.
 * Collisions are possible but not very likely (hopefully)
 * @param x String to hash
 * @return hash value
 */
uint64_t
hstring_hash1 (hstring_t *self)
{
    if (self->type == HSTRING_TYPE_BIT && self->str.c)
        return MurmurHash64B(self->str.c, sizeof(char) * self->len / 8, 0xc0ffee);
    if (self->type == HSTRING_TYPE_BYTE && self->str.c)
        return MurmurHash64B(self->str.c, sizeof(char) * self->len, 0xc0ffee);
    if (self->type == HSTRING_TYPE_TOKEN && self->str.s)
        return MurmurHash64B(self->str.s, sizeof(sym_t) * self->len, 0xc0ffee);

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
uint64_t
hstring_hash_sub (hstring_t *self, int i, int l)
{

    if (i > self->len - 1 || i + l > self->len) {
        warning("Invalid range for substring (i:%d;l:%d;x:%d)", i, l, self->len);
        return 0;
    }

    if (self->type == HSTRING_TYPE_BIT && self->str.c) {
        error("Substrings are currently not supported for bits");
        return 0;
    }

    if (self->type == HSTRING_TYPE_BYTE && self->str.c)
        return MurmurHash64B(self->str.c + i, sizeof(char) * l, 0xc0ffee);
    if (self->type == HSTRING_TYPE_TOKEN && self->str.s)
        return MurmurHash64B(self->str.s + i, sizeof(sym_t) * l, 0xc0ffee);

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
uint64_t
hstring_hash2 (hstring_t *x, hstring_t *y)
{
    uint64_t a, b;

    if (x->type == HSTRING_TYPE_BIT && y->type == HSTRING_TYPE_BIT && x->str.c && y->str.c) {
        a = MurmurHash64B(x->str.c, sizeof(char) * x->len / 8, 0xc0ffee);
        b = MurmurHash64B(y->str.c, sizeof(char) * y->len / 8, 0xc0ffee);
        return swap(a) ^ b;
    }
    if (x->type == HSTRING_TYPE_BYTE && y->type == HSTRING_TYPE_BYTE && x->str.c && y->str.c) {
        a = MurmurHash64B(x->str.c, sizeof(char) * x->len, 0xc0ffee);
        b = MurmurHash64B(y->str.c, sizeof(char) * y->len, 0xc0ffee);
        return swap(a) ^ b;
    }
    if (x->type == HSTRING_TYPE_TOKEN && y->type == HSTRING_TYPE_TOKEN && x->str.s && y->str.s) {
        a = MurmurHash64B(x->str.s, sizeof(sym_t) * x->len, 0xc0ffee);
        b = MurmurHash64B(y->str.s, sizeof(sym_t) * y->len, 0xc0ffee);
        return swap(a) ^ b;
    }

    warning("Nothing to hash. Strings are missing or incompatible.");
    return 0;
}


/**
 * Compare two symbols/characters
 * @param x string x
 * @param i position in string x
 * @param y string y
 * @param j position in string y
 * @return 0 if equal, < 0 if x smaller, > 0 if y smaller
 */
int
hstring_compare (hstring_t *x, int i, hstring_t *y, int j)
{
    assert(x->type == y->type);
    assert(i < x->len && j < y->len);
    int a, b;

    switch (x->type) {
    case HSTRING_TYPE_BIT:
        a = x->str.c[i / 8] >> (7 - i % 8) & 1;
        b = y->str.c[j / 8] >> (7 - j % 8) & 1;
        return (a - b);
    case HSTRING_TYPE_TOKEN:
        return (x->str.s[i] - y->str.s[j]);
    case HSTRING_TYPE_BYTE:
        return (x->str.c[i] - y->str.c[j]);
    default:
        error("Unknown string type");
    }
    return 0;
}


/**
 * Read in and hash stop tokens
 * @param file stop token file
 */
void
hstring_stoptokens_load(const char *file)
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
static void
stoptokens_filter (hstring_t *self)
{
    assert(self->type == HSTRING_TYPE_TOKEN);
    stoptoken_t *stoptoken;
    int i, j;

    for (i = j = 0; i < self->len; i++) {
        /* Check for stop token */
        sym_t sym = self->str.s[i];
        HASH_FIND(hh, stoptokens, &sym, sizeof(sym_t), stoptoken);

        /* Remove stoptoken */
        if (stoptoken)
            continue;

        if (i != j)
            self->str.s[j] = self->str.s[i];
        j++;
    }
    self->len = j;
}


//  --------------------------------------------------------------------------
//  Preprocess a given string
//  @param x character string
//  @return preprocessed string

void
hstring_preproc (hstring_t *self, measures_t *measure)
{
    assert(self->type == HSTRING_TYPE_BYTE);
    int decode, reverse, soundex, c, i, k;
    const char *gran;

    config_lookup_string(measure->cfg, "measures.granularity", &gran);
    config_lookup_bool(measure->cfg, "input.decode_str", &decode);
    config_lookup_bool(measure->cfg, "input.reverse_str", &reverse);
    config_lookup_bool(measure->cfg, "input.soundex", &soundex);

    if (decode) {
        self->len = decode_str(self->str.c);
        self->str.c = (char *) realloc(self->str.c, self->len);
    }

    if (reverse) {
        for (i = 0, k = self->len - 1; i < k; i++, k--) {
            c = self->str.c[i];
            self->str.c[i] = self->str.c[k];
            self->str.c[k] = c;
        }
    }

    if (soundex)
        hstring_soundex (self);

    if (!strcasecmp (gran, "bytes")) {
        /* nothing */
    } else if (!strcasecmp (gran, "tokens")) {
        assert (hstring_has_delim ());
        hstring_tokenify (self);
    } else if (!strcasecmp (gran, "bits")) {
        hstring_bitify (self);
    } else {
        error("Unknown granularity '%s'. Using 'bytes' instead.", gran);
    }

    if (stoptokens)
        stoptokens_filter (self);
}

/**
 * Destroy stop tokens table
 */
void hstring_stoptokens_destroy()
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
 * are treated as one character, if they have the same indeself-> This
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
void
hstring_soundex (hstring_t * self)
{
    int start = 0, i, alloc = 0, end = 0;
    char sdx[5], *out = NULL;

    assert(self->type == HSTRING_TYPE_BYTE);

    for (i = 0; i < self->len; i++) {
        /* Compute soundex for each substring of letters */
        if (i == self->len - 1 || !isalpha(self->str.c[i])) {
            if (start < i) {
                int len = i - start + ((i == self->len - 1) ? 1 : 0);
                soundex (self->str.c + start, len, sdx);
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
    free(self->str.c);
    self->str.c = out;
    self->len = end - 1;
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
