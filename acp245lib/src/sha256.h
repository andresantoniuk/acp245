/*-
 * Copyright (c) 2001-2003 Allan Saddi <allan@saddi.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ALLAN SADDI AND HIS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL ALLAN SADDI OR HIS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * SHA256 processing functions.
 *
 * @file sha256.h
 * @date  06/21/2009 01:11:12 PM
 */

#ifndef __sha256_h__
#define __sha256_h__

#ifdef E_ACP245_HAVE_E_LIBS
#include "e_port.h"
#else
#include "acp_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Size of a SHA256 hash, in bytes */
#define SHA256_HASH_SIZE    (32)

/** Size of a SHA256 hash, in 8 bit words */
#define SHA256_HASH_WORDS   (8)

/**
 * SHA 256 context.
 *
 * Internal representation, API users should not access
 * these fields.
 */
typedef union _SHA256Context_buffer {
    u32 words[16];
    u8 bytes[64];
} _SHA256Context_buffer;

typedef struct _SHA256Context {
    /* FIXME: totalLength might be overflown */
    u32 totalLength;
    u32 hash[SHA256_HASH_WORDS];
    u32 bufferLength;
    _SHA256Context_buffer buffer;
} SHA256Context;

/**
 * Initializes a SHA256 context.
 * @param sc the context.
 * @pre sc != NULL
 */
extern void SHA256Init(SHA256Context *sc);

/**
 * Update the current SHA256 state by adding the given data.
 * @param sc a context previously initialized with SHA256Init.
 * @param data the data
 * @param len the length of the data buffer.
 * @pre sc != NULL
 * @pre data != NULL
 */
extern void SHA256Update(SHA256Context *sc, const void *data, u32 len);

/*@-fixedformalarray@*/
/**
 * Calculates the SHA256 digest of current SHA256 context.
 * @param sc a context previously initialized with SHA256Init.
 * @param hash a byte array of length SHA256_HASH_SIZE.
 * @pre sc != NULL
 * @pre hash != NULL and hash is SHA256_HASH_SIZE bytes long.
 */
extern void SHA256Final(SHA256Context *sc, u8 hash[SHA256_HASH_SIZE]);
/*@+fixedformalarray@*/

#ifdef __cplusplus
}
#endif

#endif /* __sha256_h__ */
