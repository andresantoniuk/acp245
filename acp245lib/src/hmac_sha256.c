/*=============================================================================
  Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

  This software is furnished under a license and may be used and copied
  only in accordance with the terms of such license and with the
  inclusion of the above copyright notice. This software or any other
  copies thereof may not be provided or otherwise made available to any
  other person. No title to and ownership of the software is hereby
  transferred.
  ==============================================================================*/
/**
 * HMAC calculation functions.
 *
 * @file hmac_sha256.h
 * @date  06/23/2009 10:01:32 AM
 */
/*
 * Contributors:  Santiago Aguiar <santiago.aguiar@edantech.com>
 */
#include "acp245_config.h"

#include "hmac_sha256.h"

/*
 * Code based on http://www.faqs.org/rfcs/rfc2202.html.
 *
 * THIS SOFTWARE IS PROVIDED BY Edantech ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "hmac_sha256.h"
#include "sha256.h"
#include <stddef.h>
#include <stdio.h>

#ifndef SHA_BLOCKSIZE
#define SHA_BLOCKSIZE   64
#endif

/*
 * @param d1 data to be truncated
 * @param d2 truncated data
 * @param len length in bytes to keep
 */
static void _truncate(const u8* d1, u8* d2, u16 len) {
    u32 i;
    for (i = 0 ; i < len ; i++) {
        d2[i] = d1[i];
    }
}

/* @param k secret key */
/* @param lk length of the key in bytes */
/* @param d data */
/* @param ld length of data in bytes */
/* @param t output buffer, at least "t" bytes */
extern void hmac_sha256(const u8*  k, u16 lk, const u8* d, u16 ld, u8* out, u16 t) {
    SHA256Context ictx, octx ;
    u8 isha[SHA256_HASH_SIZE], osha[SHA256_HASH_SIZE] ;
    u8 key[SHA256_HASH_SIZE] ;
    u8 buf[SHA_BLOCKSIZE] ;
    u32 i;

    if (lk > SHA_BLOCKSIZE) {
        SHA256Context tctx ;

        SHA256Init(&tctx) ;
        SHA256Update(&tctx, k, lk) ;
        SHA256Final(&tctx, key) ;

        k = key ;
        lk = SHA256_HASH_SIZE;
    }

    /**** Inner Digest ****/

    SHA256Init(&ictx) ;

    /* Pad the key for inner digest */
    for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x36 ;
    for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x36 ;

    SHA256Update(&ictx, buf, SHA_BLOCKSIZE) ;
    SHA256Update(&ictx, d, ld) ;

    SHA256Final(&ictx, isha) ;

    /**** Outter Digest ****/

    SHA256Init(&octx) ;

    /* Pad the key for outter digest */

    for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x5C ;
    for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x5C ;

    SHA256Update(&octx, buf, SHA_BLOCKSIZE) ;
    SHA256Update(&octx, isha, SHA256_HASH_SIZE) ;

    SHA256Final(&octx, osha) ;

    /* truncate and print the results */
    t = t > SHA256_HASH_SIZE ? SHA256_HASH_SIZE : t ;
    _truncate(osha, out, t) ;
}
