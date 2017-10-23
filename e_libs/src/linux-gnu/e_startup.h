/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#ifndef __e_startup_h_
#define __e_startup_h_

#include "e_port.h"

#ifdef E_LIBS_STARTUP_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*e_startup_callback) (void);

extern void e_startup(e_startup_callback cb, e_startup_callback endcb);

#ifdef __cplusplus
}
#endif

#endif /* E_LIBS_STARTUP_ENABLE */

#endif
