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
 * @date  06/23/2009 10:01:28 AM
 */
/*
 * Contributors:  Santiago Aguiar <santiago.aguiar@edantech.com>
 */
#ifndef __hmac_sha256_h__
#define __hmac_sha256_h__

#ifdef E_ACP245_HAVE_E_LIBS
#include "e_port.h"
#else
#include "acp_types.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculates the HMAC-SHA256 function of the given byte array.
 *
 * Calculates out = HMAC-SHA256(k, d). If out is too small to
 * store the result, the output will be truncated.
 *
 * @param k secret key
 * @param lk length of the key in bytes
 * @param d data
 * @param ld length of data in bytes
 * @param t output buffer, at least "t" bytes
 * @pre k != NULL
 * @pre d != NULL
 * @pre out != NULL
 * @post out = HMAC-SHA256(k, d)
 */
extern void hmac_sha256(const u8*  k, u16 lk, const u8* d, u16 ld, u8* out, u16 t);

#ifdef __cplusplus
}
#endif

#endif /* __hmac_sha256_h__ */

