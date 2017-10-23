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

#include "e_io.h"
#include "e_io_wm.h"

#include "adl_global.h"

static adl_port_e _stderr_port = ADL_PORT_NONE;

void e_io_puts_stderr(ascii* buff) {
    (void) adl_atSendResponse( ADL_AT_PORT_TYPE ( _stderr_port, ADL_AT_RSP ), buff);
}

void e_io_wm_set_stderr_at_port(adl_port_e port){
    _stderr_port = port;
}
