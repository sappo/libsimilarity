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


#ifndef UTIL_H
#define UTIL_H

/* Fatal message */
#ifndef fatal
#define fatal(...)     {err_msg("Error", __func__, __VA_ARGS__); exit(-1);}
#endif
/* Error message */
#ifndef error
#define error(...)     {err_msg("Error", __func__, __VA_ARGS__);}
#endif
/* Warning message */
#ifndef warning
#define warning(...)   {err_msg("Warning", __func__, __VA_ARGS__);}
#endif

/* Utility functions */
void err_msg(char *, const char *, char *, ...);
void info_msg(int, char *, ...);
double time_stamp();
void prog_bar(vcache_t *cache, long, long, long);
size_t gzgetline(char **s, size_t * n, gzFile f);
void strtrim(char *x);
int decode_str(char *str);
uint64_t hash_str(char *s, int l);
int strip_newline(char *s, int l);
void debug_msg(char *m, ...);
void log_print(vcache_t *cache, long, long, long);
float hround(float, int);

#if !defined (MIN)
#define MIN(a, b) (a < b ? a : b)
#endif
#if !defined (MAX)
#define MAX(a, b) (a > b ? a : b)
#endif

#define UNUSED(x) (void)(x)

void
    util_test (bool verbose);
#endif /* UTIL_H */
