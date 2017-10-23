/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"

#include "e_errors_wip.h"

#include "adl_global.h"
#include "wip.h"

#include "e_port.h"
#include "e_log.h"

/**
 * WARNING:
 * This file has very special semantics to avoid code duplication if possible.
 * The e_log E_* macros are redefined to use the file and line provider by parameter
 * in all of the err_log_* functions.
 *
 * Therefore, while inside this file, E_* macros can only be used inside an err_log*
 * function which declares a file and line parameter.
 */
#ifdef __e_log_h_MACROS
#undef E_LOG
#define E_LOG(x, ...)		if(e_log_errlevel >= x) \
        { e_log_file(file, line, x, __VA_ARGS__ ); }
#endif
void err_log_wip_bearerOpen(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case WIP_BERR_NO_DEV:
        E_FATAL("Bearer open failed. The device does not exist.");
        break;
    case WIP_BERR_ALREADY:
        E_ERR("Bearer open failed. The device is already opened.");
        break;
    case WIP_BERR_NO_IF:
        E_ERR("Bearer open failed. Network interface is not available.");
        break;
    case WIP_BERR_NO_HDL:
        E_ERR("Bearer open failed. No free handle.");
        break;
    default:
        E_FATAL("Bearer open failed. Unknwon error: 0x%hx", (short) rc);
        break;
    }
}

void err_log_wip_bearerClose(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case WIP_BERR_BAD_HDL:
        E_FATAL("Bearer close failed. Bad Handler.");
        break;
    case WIP_BERR_BAD_STATE:
        E_ERR("Bearer close failed. Bad State.");
        break;
    default:
        E_FATAL("Bearer close failed. Unknwon error: 0x%hx", (short) rc);
        break;
    }
}

void err_log_wip_bearerStart(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case WIP_BERR_OK_INPROGRESS:
        E_WARN("Bearer start OK. Start is in progress.");
        break;
    case WIP_BERR_BAD_HDL:
        E_FATAL("Bearer start failed. Bad Handler.");
        break;
    case WIP_BERR_BAD_STATE:
        E_FATAL("Bearer start failed. The bearer is not stopped.");
        break;
    case WIP_BERR_DEV:
        E_FATAL("Bearer start failed. Error from link layer init.");
        break;
    default:
        E_FATAL("Bearer start failed. Unknwon error: 0x%hx", (short) rc);
        break;
    }
}

void err_log_wip_bearerStop(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case WIP_BERR_OK_INPROGRESS:
        E_WARN("Bearer stop failed. Another disconnection is in progress.");
        break;
    case WIP_BERR_BAD_HDL:
        E_FATAL("Bearer stop failed. Bad Handler.");
        break;
    default:
        E_FATAL("Bearer stop failed. Unknwon error: 0x%hx", (short) rc);
        break;
    }
}

void err_log_wip_bearerSetOpts(const char* file, const long line, s8 rc)
{
    switch(rc) {
    case WIP_BERR_BAD_HDL:
        E_WARN("Bearer set options failed. Bad Handler.");
        break;
    case WIP_BERR_OPTION:
        E_FATAL("Bearer set options failed. Invalid option.");
        break;
    case WIP_BERR_PARAM:
        E_FATAL("Bearer set options failed. Invalid option value.");
        break;
    default:
        E_FATAL("Bearer set options failed. Unknwon error: 0x%hx", (short) rc);
        break;
    }
}

void err_log_wip_setOpts(const char* file, const long line, int rc)
{
    switch(rc) {
    case WIP_CERR_NOT_SUPPORTED:
        E_WARN("Unsupported option.");
        break;
    case WIP_CERR_INVALID:
        E_FATAL("Invalid option.");
        break;
    default:
        E_FATAL("wip_setOpts failed. Unknwon error: 0x%x", rc);
        break;
    }
}

void err_log_wip_close(const char* file, const long line, int rc) {
    switch(rc) {
    case WIP_CERR_MEMORY:
        E_FATAL("wip_close: Insufficient memory to queue channel.");
        break;
    case WIP_CERR_INVALID:
        E_FATAL("wip_close: NULL channel specified.");
        break;
    default:
        E_FATAL("wip_close: Unknwon error: 0x%x", rc);
        break;
    }
}

void err_log_wip_shutdown(const char* file, const long line, int rc) {
    switch(rc) {
    case WIP_CERR_NOT_SUPPORTED:
        E_FATAL("wip_shutdown: shutdown not supported on channel");
        break;
    case WIP_CERR_INTERNAL:
        E_FATAL("wip_shutdown: Impossible to abort the TCP communication");
        break;
    default:
        E_FATAL("wip_shutdown: Unknwon error: 0x%x", rc);
        break;
    }
}

void err_log_wip_write(const char* file, const long line, int rc) {
    switch(rc) {
    case WIP_CERR_CSTATE:
        E_ERR("wip_write: the channel is not ready to write data");
        break;
    case WIP_CERR_NOT_SUPPORTED:
        E_FATAL("wip_write: the channel does not support writes");
        break;
    default:
        E_FATAL("wip_write: Unknwon error: 0x%x", rc);
        break;
    }
}

void err_log_wip_read(const char* file, const long line, int rc) {
    switch(rc) {
    case WIP_CERR_CSTATE:
        E_ERR("wip_read: the channel is not ready to read data");
        break;
    case WIP_CERR_NOT_SUPPORTED:
        E_FATAL("wip_read: the channel does not support reads");
        break;
    default:
        E_FATAL("wip_read: Unknwon error: 0x%x", rc);
        break;
    }
}
