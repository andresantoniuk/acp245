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
 * ACP License verification.
 * @date 09/01/2009 04:10:19 PM
 * @file acp_license.h
 * @author Edantech
 * @internal
 */
/* Contributors:
 * Santiago Aguiar, santiago.aguiar@edantech.com
 */
#ifndef __acp_license_h__
#define __acp_license_h__

/** @cond EXPORT (doxygen cond) */

/* Definitions for DLL exports for WIN32 platforms. */
#if defined _MSC_VER || defined _WIN32
#ifdef E_ACP245_EXPORT_DLL
#define E_EXPORT __declspec (dllexport) extern
#else
#define E_EXPORT __declspec (dllimport) extern
#endif /* E_EXPORT_DLL */
#else
#define E_EXPORT extern
#endif /* _MSC_VER */

/** @endcond */


#ifdef E_ACP245_HAVE_E_LIBS
/* Include e_libs only if packed with the ACP library. */
#include "e_port.h"
#else
#include "acp_types.h"
#endif

#define ACP_LICENSE_VALID                       ((e_ret)0)
#define ACP_LICENSE_NO_LICENSE                  ((e_ret)-1)
#define ACP_LICENSE_INVALID_FORMAT              ((e_ret)-2)
#define ACP_LICENSE_INVALID                     ((e_ret)-3)

#ifdef __cplusplus
extern "C" {
#endif

E_EXPORT e_ret acp_license_verify(const char *license_filename);

E_EXPORT bool acp_license_verified(void);

#ifdef __cplusplus
}
#endif

#endif /* __acp_license_h__ */

