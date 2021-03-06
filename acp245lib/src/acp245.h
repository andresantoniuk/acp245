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
 * ACP 245 message library main header.
 *
 * Include this header to use the library in your own code.
 *
 * @file acp245.h
 * @date  03/13/2009 01:42:35 PM
 * @author Edantech
 * @see acp_msg.h
 * @see acp_el.h
 */
/* Contributors:  Santiago Aguiar, santiago.aguiar@edantech.com */
#ifndef __acp245_h__
#define __acp245_h__

/** ACP245 library version number */
#define ACP245_VERSION  1.6.3

/**
 * @mainpage ACP245 message library.
 * @section Introduction
 * This library provides a portable implementation of the ACP245 protocol
 * messages.
 *
 * To use the library, you should include the @ref acp245.h header
 * file and link agains the provided library binaries.
 *
 * The library does not include any network related code, only functions to
 * read, write and validate ACP245 messages. An ACP245 server or client can be
 * built by using this library to process the binary messages.
 *
 * The main functions are:
 * @li @ref acp_msg_read_data : reads an ACP message from a byte array.
 * @li @ref acp_msg_write_data : writes an ACP message to a byte array.
 *
 * Both functions operate on an @ref acp_msg structure which includes a the
 * application ID and message type of the ACP message. Based on that
 * application ID and type, different fields are available on the data
 * field of the @ref acp_msg structure.
 *
 * The following code illustrates a simple use of the API to read and write
 * an empty Alarm Keepalive:
 * @code
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include "acp245.h"
 * int main(int argc, char** argv) {
 *      u8 buf[256];
 *      u32 readed;
 *      u32 written;
 *      acp_msg msg;
 *      acp_msg msg_read;
 *      e_ret rc;
 *
 *      acp_msg_init(&msg, ACP_APP_ID_ALARM, ACP_MSG_TYPE_ALARM_KA);
 *
 *      rc = acp_msg_write_data(buf, 256, &written, &msg);
 *      if (ACP_MSG_OK == rc) {
 *          printf("Written OK.\n");
 *      }
 *
 *      rc = acp_msg_read_data(buf, written, &readed, &msg_read);
 *      if(ACP_MSG_OK == rc) {
 *          printf("ACP Message Application Id is: %x\n", msg_read.hdr.app_id);
 *          printf("ACP Message Type is: %x\n", msg_read.hdr.type);
 *      }
 *
 *      getchar();
 * }
 * @endcode
 *
 * Structures and functions make reference to the following documents:
 * @li [ACP245]: ACP 245 v1.2.2, Protocol Specification, 14/08/09.@n
 * http://www.denatran.gov.br/download/ACP%20245%20V%201.2.2%2014_08_09%2013_46.pdf.
 * Also included in project documentation.
 * @li [ACP] ACP v. 3.0.1, March 2000. Included in project documentation.
 */

#include "acp_init.h"
#include "acp_msg.h"
#include "acp_el.h"

#endif /* __acp245_h__ */
