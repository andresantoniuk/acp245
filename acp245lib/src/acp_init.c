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
 * Library initialization functions.
 * @date 09/03/2009 04:29:31 PM
 * @file acp_init.c
 * @author Edantech
 */
/* Contributors:
 * Santiago Aguiar, santiago.aguiar@edantech.com
 */
#include "acp245_config.h"

#include "acp_init.h"

#include <stdlib.h>

#include "e_util.h"

#include "acp_license.h"

e_ret acp_init(void) {
    if (e_file_access(ACP_INIT_DEFAULT_LICENSE)) {
        return acp_init_opts(ACP_INIT_DEFAULT_LICENSE);
    } else {
        char *license_file = getenv(ACP_INIT_ENV_LICENSE_FILE);
        if (license_file != NULL) {
            return acp_init_opts(license_file);
        } else {
            /* let the default implementation handle the not
             * found case... */
            return acp_init_opts(ACP_INIT_DEFAULT_LICENSE);
        }
    }
}

e_ret acp_init_opts(const ascii * license_filename, ...) {
    return acp_license_verify(license_filename);
}
