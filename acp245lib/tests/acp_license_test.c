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
 *
 * @date 09/03/2009 12:02:27 PM
 * @file acp_license_test.c
 * @author Edantech
 */
/* Contributors:
 * Santiago Aguiar, santiago.aguiar@edantech.com
 */
#include "acp245_config.h"
#include "acp_license.h"

#include "acp245_config.h"
#include "acp_key.h"

#include "e_check.h"
#include "e_log.h"

START_TEST (test_acp_license_verify)
{
    e_ret rc;
    rc = acp_license_verify("unexistent_license.sig");
    ARE_EQ_INT(ACP_LICENSE_NO_LICENSE, rc);

    rc = acp_license_verify("valid_license.sig");
    ARE_EQ_INT(ACP_LICENSE_VALID, rc);

    rc = acp_license_verify("invalid_license.sig");
    ARE_EQ_INT(ACP_LICENSE_INVALID, rc);

    rc = acp_license_verify("invalid_format_license.sig");
    ARE_EQ_INT(ACP_LICENSE_INVALID_FORMAT, rc);
}
END_TEST

extern int acp_license_test (void);
int acp_license_test (void) {
    return e_check_run_suite("acp_license",
            test_acp_license_verify,
            NULL);
}

#ifndef USE_SINGLE_TEST
int main (void) {
	return acp_license_test();
}
#endif
