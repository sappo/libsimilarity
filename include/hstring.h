/*  =========================================================================
    hstring - String object with functions for processing strings and sequences

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.  This program is distributed without any
    warranty. See the GNU General Public License for more details.
    =========================================================================
*/

#ifndef HSTRING_H_INCLUDED
#define HSTRING_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @warning THE FOLLOWING @INTERFACE BLOCK IS AUTO-GENERATED BY ZPROJECT
//  @warning Please edit the model at "api/hstring.api" to make changes.
//  @interface
//  This API is a draft, and may change without notice.
#ifdef HARRY_BUILD_DRAFT_API
#define HSTRING_DELIM_NOT_INIT 42           // Placeholder for non-initialized delimiters
#define HSTRING_TYPE_BYTE 0x00              // String type Byte
#define HSTRING_TYPE_TOKEN 0x01             // String type Token
#define HSTRING_TYPE_BIT 0x02               // String type Bit

//  *** Draft method, for development use, may change without warning ***
//  Converts a c-style string into a string object.
HARRY_EXPORT hstring_t *
    hstring_new (const char *str);

//  *** Draft method, for development use, may change without warning ***
//  Creates an empty string object.
HARRY_EXPORT hstring_t *
    hstring_empty (int type);

//  *** Draft method, for development use, may change without warning ***
//  Destroy the string object.
HARRY_EXPORT void
    hstring_destroy (hstring_t **self_p);

//  *** Draft method, for development use, may change without warning ***
//  Decodes a string containing delimiters into a lookup table.
HARRY_EXPORT void
    hstring_delim_set (const char *str);

//  *** Draft method, for development use, may change without warning ***
//  Check whether delimiters have been set. Returns true if delimiters have
//  been set, otherwise false.                                             
HARRY_EXPORT int
    hstring_has_delim (void);

//  *** Draft method, for development use, may change without warning ***
//  Resets delimiters table. There is a global table of delimiter symbols
//  which is only initialized once the first sequence is processed. This 
//  functions is used to trigger a re-initialization.                    
HARRY_EXPORT void
    hstring_delim_reset (void);

//  *** Draft method, for development use, may change without warning ***
//  
HARRY_EXPORT void
    hstring_preproc (hstring_t *self, measures_t *measure);

//  *** Draft method, for development use, may change without warning ***
//  Converts a string into a sequence of tokens using delimiter characters.
//  The original character string is lost. Returns 0 if successful,        
//  otherwise -1.                                                          
HARRY_EXPORT int
    hstring_tokenify (hstring_t *self);

//  *** Draft method, for development use, may change without warning ***
//  Converts a string into a sequence of bits. Well, actually there is no
//  conversion except for that the counting now happens on the level of  
//  bits instead of bytes.                                               
HARRY_EXPORT void
    hstring_bitify (hstring_t *self);

//  *** Draft method, for development use, may change without warning ***
//  Computes a 64-bit hash for a substring. Collisions are possible but not
//  very likely (hopefully). Returns 64-bit hash.                          
HARRY_EXPORT uint64_t
    hstring_hash_sub (hstring_t *self, int start, int len);

//  *** Draft method, for development use, may change without warning ***
//  Computes a 64-bit hash for a string. The hash is used at different  
//  locations.  Collisions are possible but not very likely (hopefully).
//  Returns 64-bit hash.                                                
HARRY_EXPORT uint64_t
    hstring_hash1 (hstring_t *self);

//  *** Draft method, for development use, may change without warning ***
//  Computes a 64-bit hash for two strings. The computation is symmetric,  
//  that is, the same strings retrieve the same hash independent of their  
//  order.  Collisions are possible but not very likely (hopefully). Return
//  64-bit hash.                                                           
HARRY_EXPORT uint64_t
    hstring_hash2 (hstring_t *x, hstring_t *y);

//  *** Draft method, for development use, may change without warning ***
//  Returns symbol/character at given position.
HARRY_EXPORT uint64_t
    hstring_get (hstring_t *self, int pos);

//  *** Draft method, for development use, may change without warning ***
//  Perform a soundex transformation of each token.                       
//                                                                        
//  Soundex code as implemented by Kevin Setter, 8/27/97 with some slight 
//  modifications. Known bugs: Consonants separated by a vowel are treated
//  as one character, if they have the same index. This is wrong. :(      
HARRY_EXPORT void
    hstring_soundex (hstring_t *self);

//  *** Draft method, for development use, may change without warning ***
//  Compares two symbols/characters. Returns 0 if equal, < 0 if x smaller,
//  > 0 if y smaller                                                      
HARRY_EXPORT int
    hstring_compare (hstring_t *x, int posx, hstring_t *y, int posy);

//  *** Draft method, for development use, may change without warning ***
//  Print string object
HARRY_EXPORT void
    hstring_print (hstring_t *self);

//  *** Draft method, for development use, may change without warning ***
//  Reads stop tokens from file and hash them.
HARRY_EXPORT void
    hstring_stoptokens_load (hstring_t *self, const char *filename);

//  *** Draft method, for development use, may change without warning ***
//  Destroys stop tokens table.
HARRY_EXPORT void
    hstring_stoptokens_destroy (hstring_t *self);

//  *** Draft method, for development use, may change without warning ***
//  Self test of this class.
HARRY_EXPORT void
    hstring_test (bool verbose);

#endif // HARRY_BUILD_DRAFT_API
//  @end

#ifdef __cplusplus
}
#endif

#endif
