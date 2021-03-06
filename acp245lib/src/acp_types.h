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
 * ACP 245 primitive type definitions.
 *
 * This file will be included by the ACP 245 library if not packed with e_libs,
 * otherwise the standard e_libs type definition file will be used instead.
 *
 * @file acp_types.h
 * @date  05/02/2009 03:22:18 PM
 * @author Edantech
 */
/*
 *   Contributors:  Santiago Aguiar, santiago.aguiar@edantech.com
 */
#ifndef __acp_types_h__
#define __acp_types_h__

/**
 * @name Primitive type definitions.
 */
/*@{*/
#ifdef __GNUC__

#ifndef __cplusplus
typedef unsigned char   bool;
typedef unsigned char   u8;
typedef signed   char   s8;
typedef          char   ascii;
typedef unsigned short  u16;
typedef          short  s16;
typedef unsigned int    u32;
typedef          int    s32;
#else
typedef unsigned char			bool;
typedef unsigned _int8          u8;
typedef unsigned _int16         u16;
typedef unsigned _int32         u32;
typedef unsigned _int64         u64;
typedef          char           ascii;
typedef _int8                   s8;
typedef _int16                  s16;
typedef _int32                  s32;
typedef _int64                  s64;
#define huge
#endif

#elif defined _MSC_VER

#if (_MSC_VER < 1300)
   typedef unsigned char	 bool;
   typedef char              s8;
   typedef short             s16;
   typedef int               s32;
   typedef          char     ascii;
   typedef unsigned char     u8;
   typedef unsigned short    u16;
   typedef unsigned int      u32;
#else
   typedef unsigned char	 bool;
   typedef __int8            s8;
   typedef __int16           s16;
   typedef __int32           s32;
   typedef char              ascii;
   typedef unsigned __int8   u8;
   typedef unsigned __int16  u16;
   typedef unsigned __int32  u32;
#endif

#endif
/*@}*/

typedef s16 e_ret;

#endif /* __acp_types_h__ */

