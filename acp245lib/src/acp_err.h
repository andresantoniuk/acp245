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
 * ACP 245 error codes.
 *
 * @file acp_err.h
 * @date  03/13/2009 09:12:03 PM
 * @author Edantech
 */
/*
 * Contributors:  Santiago Aguiar <santiago.aguiar@edantech.com>
 */
#ifndef __acp_err_h_
#define __acp_err_h_

/* message format errors */

#define ACP_MSG_OK                      0x0000
#define ACP_MSG_ERR_TOO_SHORT           0x8001
#define ACP_MSG_ERR_TOO_LONG            0x8002
#define ACP_MSG_ERR_INCOMPLETE          0x8003

#define ACP_MSG_ERR_BAD_FORMAT          0x8004
#define ACP_MSG_ERR_BAD_LENGTH          0x8005
#define ACP_MSG_ERR_INVALID_DEFAULT     0x8006

#define ACP_MSG_ERR_UNKNOWN_MSG_TYPE    0x8007
#define ACP_MSG_ERR_UNKNOWN_APP_ID      0x8008

#define ACP_MSG_ERR_UNSUPPORTED         0x80A0
#define ACP_MSG_ERR_UNSUP_MSG_TYPE      0x80A1

#define ACP_MSG_ERR_NO_MEM              0x80FE
#define ACP_MSG_ERR_FATAL               0x80FF

#endif
