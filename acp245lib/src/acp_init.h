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
 * Library initilization functions.
 * @date 09/03/2009 04:28:31 PM
 * @file acp_init.h
 * @author Edantech
 */
/* Contributors:
 * Santiago Aguiar, santiago.aguiar@edantech.com
 */
#ifndef __acp_init_h__
#define __acp_init_h__

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

#ifdef __cplusplus
extern "C" {
#endif

/** Library initialized OK */
#define ACP_INIT_OK                  (0)
/** Invalid license. Library can not be initialized. */
#define ACP_INIT_INVALID_LICENSE     (-1)
/** Generic initialization error. */
#define ACP_INIT_ERROR               (-2)

/** Default license filename. Path is relative to working directory. */
#define ACP_INIT_DEFAULT_LICENSE     "license.sig"

/** Name of the environment variable that may hold the license file name */
#define ACP_INIT_ENV_LICENSE_FILE    "E_ACP245_LICENSE"

/** Initialization options.*/
typedef enum acp_init_opt {
    /** This value must be the last parameter when using initialization
     * options */
    ACP_INIT_END
} acp_init_opt;

/**
 * Initialize library with default arguments.
 *
 * The function will first check for a valid license on
 * @ref ACP_INIT_DEFAULT_LICENSE and if it that file doesn't exists, it will then
 * check on the value of the environment variable referenced by
 * @ref ACP_INIT_ENV_LICENSE_FILE.
 *
 * @return ACP_INIT_OK if library was successfully initialized.
 *         ACP_INIT_INVALID_LICENSE if the library license is invalid.
 *         ACP_INIT_ERROR if there's another error initializing the library.
 */
E_EXPORT e_ret acp_init(void);

/**
 * Initializes the library with the given initialization options.
 *
 * The options must be sent in the format:
 * @ref acp_init_opt code, &lt;value&gt;
 *
 * At the current time, no options are supported.
 * @param license_filename the file name of the license file to use for
 * license verification.
 *
 * @return ACP_INIT_OK if library was successfully initialized.
 *         ACP_INIT_INVALID_LICENSE if the library license is invalid.
 *         ACP_INIT_ERROR if there's another error initializing the library.
 */
E_EXPORT e_ret acp_init_opts(const ascii * license_filename, ...);

#ifdef __cplusplus
}
#endif

#endif /* __acp_init_h__ */

