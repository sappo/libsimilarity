/*  =========================================================================
    hmatrix - class description

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the  
    Free Software Foundation; either version 3 of the License, or (at your 
    option) any later version.  This program is distributed without any    
    warranty. See the GNU General Public License for more details.         
    =========================================================================
*/

#ifndef HMATRIX_H_INCLUDED
#define HMATRIX_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new hmatrix
HARRY_EXPORT hmatrix_t *
    hmatrix_new (void);

//  Destroy the hmatrix
HARRY_EXPORT void
    hmatrix_destroy (hmatrix_t **self_p);

//  Self test of this class
HARRY_EXPORT void
    hmatrix_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
