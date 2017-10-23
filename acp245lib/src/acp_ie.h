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
 * ACP 245 generic information element description and processing functions.
 *
 * This file defines the structure of generic ACP information elements,
   and provides functions to read and write generic information elements
 * from byte buffers.
 *
 * A generic information element is used when the library does not know the
 * exact type of information elements that must be processed. Otherwise, a
 * function from acp_el.h should be used instead.
 *
 * The functions exported by this file are not generally useful to external
 * applications. Users of the ACP 245 library should use the functions exported on
 * acp_msg.h instead of this one.
 *
 * @file acp_ie.h
 * @date  03/13/2009 02:12:18 PM
 * @author Edantech
 */
/*
 * Contributors:  Santiago Aguiar <santiago.aguiar@edantech.com>
 */
#ifndef __acp_ie_h_
#define __acp_ie_h_

#ifdef E_ACP245_HAVE_E_LIBS
#include "e_port.h"
#include "e_buff.h"
#else
#include "acp_types.h"
#endif

#include "acp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum IE length supported */
#define ACP_IE_MAX_LEN                        ((u16)(0xFFFF))

/**
 * @name Information Element IDs
 * @see Section 1.5 of [ACP245]
 */
/*@{*/
#define ACP_IE_BINARY                         0
#define ACP_IE_ISO_8859_1                     1
#define ACP_IE_PACKED_DEC                     2
#define ACP_IE_EXTENDED                       3

/* Extended Identification elements ids */
#define ACP_IE_EXT_BINARY                     0
#define ACP_IE_EXT_ISO_8859_1                 1
#define ACP_IE_EXT_PACKED_DEC                 2
#define ACP_IE_EXT_RESERVED                   3
#define ACP_IE_EXT_UNICODE                    4
#define ACP_IE_EXT_UTF8                       5
#define ACP_IE_EXT_SHIFT_JIS                  6
/*
 * 7..30 are reserved
 * */
#define ACP_IE_EXT_PRIVATE                   31
/*@}*/

typedef union acp_ie_any_data {
    /* TODO: figure out a way to optimize cases of single 1-2-4 bytes values */
    u8 *bin;
    ascii *str;
} acp_ie_any_data;
/**
 * An information element of undetermined type.
 * This structure is used to represent an element
 * whose type has not be constrained by the ACP245
 * specification, and therefore can be represented
 * with different data types.
 */
typedef struct acp_ie_any {
    /** If TRUE, the element has been included on the message,
     * if FALSE, the element is not included because the message
     * was truncated or was explicitely excluded with a control flag. */
    bool present;
    u8 id;
    u16 len;
    /** Data of the information element.
     * str will be valid only if id == ACP_IE_ISO_8859_1 or
     * id == ACP_IE_PACKED_DEC, otherwise bin will have
     * the byte array representing the information element data.
     */
    acp_ie_any_data data;
} acp_ie_any;

#ifdef E_ACP245_HAVE_E_LIBS
/**
 * A generic uninterpreted information element.
 *
 * This structure is used internally to split the ACP message
 * in information elements composed by e_buff slices of the original
 * ACP message buffer. After the library reads an information element
 * it consumes data from it's e_buff until the information element
 * is processed.
 * @internal
 */
typedef struct acp_ie {
    u8 id;
    u16 len;
    e_buff data;
} acp_ie;

#define IE_EXIST(x)                 (IE_LEN((x))>0)
#define IE_LEN(x)                   ((x).len)
#define IE_REMAIN(x)                ((u16)(e_buff_read_remain(&((x).data))))

/**
 * Returns the number of bytes required to store a binary IE with the given data
 * length.
 * Besides the data length, an IE requires 1 byte for the header and additional
 * bytes to store the length of the IE if it exceeds 2**5.
 *
 * @param val_len the length of the IE data.
 * @return the length required to store an IE with that data length.
 * @internal
 */
extern u32 acp_ie_get_len(u32 val_len);

/**
 * Returns the number of bytes required to store a ISO-8859-1 IE with the
 * given text.
 * Besides the text, an IE requires 1 byte for the header and additional
 * bytes to store the length of the IE if it exceeds 2**5.
 *
 * @param text the text of the IE.
 * @return the length required to store a ISO-8859-1 IE with that text.
 * @internal
 */
extern u32 acp_ie_get_iso_8859_1_len(ascii* text);

/**
 * Returns the number of bytes required to store a BCD IE with the given number.
 * Besides the number, an IE requires 1 byte for the header and additional
 * bytes to store the length of the IE if it exceeds 2**5.
 * @param bcd the number to store in the IE as BCD.
 * @return the length required to store a BCD IE with that text.
 * @internal
 */
extern u32 acp_ie_get_bcd_len(ascii* bcd);

/**
 * Writes an information element header into a buffer.
 *
 * An information element header is composed by it's type and length. The
 * library handles the use of the more flag as needed.
 *
 * @param buff the buffer
 * @param type the type of the information element.
 * @param len the length of the information element.
 * @internal
 */
extern e_ret acp_ie_write_hdr(e_buff *buff, u8 type, u32 len);

/**
 * Reads a generic information element from the given buffer.
 *
 * @param buff the buffer
 * @param ie the information element
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the IE length field exceeds
 *          ACP_IE_MAX_LEN.
 * @internal
 */
extern e_ret acp_ie_read(e_buff *buff, acp_ie *ie);

/**
 * Reads a generic information element from the given buffer.
 *
 * If the information element type doesn't match the expected one, an
 * error will be returned.
 *
 * This function is used when the caller know which should be
 * the type of the information element.
 *
 * @param buff the buffer
 * @param ie the information element where to store the readed data.
 * @param id_exp the expected information element id
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the IE length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the expected value does not match.
 * @internal
 */
extern e_ret acp_ie_read_exp(e_buff *buff, acp_ie *ie, u8 id_exp);

/**
 * Reads a generic information element from the given buffer.
 *
 * If the information element type doesn't match the expected one, an
 * error will be returned.
 *
 * If the length of the element doesn't match the expected one, an
 * error will be returned.
 *
 * This function is used when the caller know which should be
 * the type and length of the information element.
 *
 * @param buff the buffer
 * @param ie the information element where to store the readed data.
 * @param id_exp the expected information element id
 * @param len_exp the expected length of the information element
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the IE length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the expected value does not match.
 * @internal
 */
extern e_ret acp_ie_read_exp_l(e_buff *buff, acp_ie *ie, u8 id_exp, u32 len_exp);

/*
 * Reads a single byte binary IE.
 *
 * @param buff the e_buff to read the IE from.
 * @param b on success, will contain the byte contained in the IE.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the IE length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the IE type doesn't match the
 *          expected type or the length is not 1 byte.
 * @pre buff != NULL
 * @pre b != NULL
 * @internal
 */
extern e_ret acp_ie_read_byte(e_buff *buff, u8 *b);

/*
 * Writes a single byte binary IE.
 *
 * @param buff the e_buff to read the IE from.
 * @param b the byte to store in the IE.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't have enough space to 
 *          store the IE.
 * @pre buff != NULL
 * @internal
 */
extern e_ret acp_ie_write_byte(e_buff *buff, u8 b);

/*
 * Reads a binary IE and allocates memory to store it.
 *
 * @param buff the e_buff to read the IE from.
 * @param data on success, will contain a pointer to the readed IE data, or
 * NULL if the IE was empty.
 * @param data_len on success, will store the length of the readed IE data.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the message length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the IE type doesn't match the
 *          expected type (binary)
 *          ACP_MSG_ERR_NO_MEM if there's not enough memory to store the IE.
 * @pre buff != NULL
 * @pre data != NULL
 * @pre data_len != NULL
 * @post ret != OK || (len == 0 && data == NULL) || (len > 0 && data != NULL)
 * @internal
 */
extern e_ret acp_ie_read_bin(e_buff *buff, u8** data, u32* data_len);

/*
 * Reads a binary IE and stores it a preallocated buffer of the required size.
 * The preallocated buffer must have at least data_len bytes.
 *
 * @param buff the e_buff to read the IE from.
 * @param data the preallocated buffer.
 * @param data_len the expected length of the IE. data must be at least
 * data_len bytes in length.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the message length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the IE type doesn't match the
 *          expected type or the IE type length is different from data_len.
 * @pre buff != NULL
 * @pre data != NULL
 * @internal
 */
extern e_ret acp_ie_put_bin(e_buff *buff, u8* data, u32 data_len);

/*
 * Writes a binary IE in the given buffer.
 *
 * @param buff the e_buff to write the IE to.
 * @param data the binary data to write.
 * @param data_len the binary data size.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't have enough space to
 *          store the IE.
 *          ACP_MSG_ERR_TOO_LONG if the space required to store the IE exceeds
 *          ACP_IE_MAX_LEN.
 * @pre buff != NULL
 * @pre data != NULL || len == 0
 * @internal
 */
extern e_ret acp_ie_write_bin(e_buff *buff, u8* data, u32 len);

/*
 * Reads an ISO-8859-1 IE and allocates memory to store it.
 *
 * @param buff the e_buff to read the IE from.
 * @param data on success, will contain a pointer to the readed NULL.
 * terminated IE data, or NULL if the IE was empty.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the message length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the IE type doesn't match the
 *          expected type (ISO-8859-1)
 *          ACP_MSG_ERR_NO_MEM if there's not enough memory to store the IE.
 * @pre buff != NULL
 * @pre data != NULL
 * @post ret != OK || data != NULL
 * @internal
 */
extern e_ret acp_ie_read_iso_8859_1(e_buff *buff, ascii** data);

/*
 * Writes a ISO-8859-1 NULL terminated string in the given buffer.
 *
 * @param buff the e_buff to write the IE to.
 * @param data a NULL terminated string.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't have enough space to
 *          store the IE.
 *          ACP_MSG_ERR_TOO_LONG if the space required to store the IE exceeds
 *          ACP_IE_MAX_LEN.
 * @pre buff != NULL
 * @internal
 */
extern e_ret acp_ie_write_iso_8859_1(e_buff *buff, ascii* data);

/*
 * Reads an BCD packed decimal IE and allocates memory to store it.
 *
 * @param buff the e_buff to read the IE from.
 * @param data on success, will contain a pointer to the readed NULL.
 * terminated IE data, or NULL if the IE was empty.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't include the full
 *          message.
 *          ACP_MSG_ERR_TOO_LONG if the message length field exceeds
 *          ACP_IE_MAX_LEN.
 *          ACP_MSG_ERR_INVALID_DEFAULT if the IE type doesn't match the
 *          expected type
 *          ACP_MSG_ERR_NO_MEM if there's not enough memory to store the IE.
 * @pre buff != NULL
 * @pre data != NULL
 * @post ret != OK || data != NULL
 * @internal
 */
extern e_ret acp_ie_read_bcd(e_buff *buff, ascii** data);

/*
 * Writes a BCD packed decimal in the given buffer.
 *
 * @param buff the e_buff to write the IE to.
 * @param data a NULL terminated string containg decimal digits.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if the buffer doesn't have enough space to
 *          store the IE.
 *          ACP_MSG_ERR_TOO_LONG if the space required to store the IE exceeds
 *          ACP_IE_MAX_LEN.
 * @pre buff != NULL
 * @internal
 */
extern e_ret acp_ie_write_bcd(e_buff *buff, ascii* data);

/*
 * Reads a byte array from the given IE.
 *
 * The function allocates memory for the byte array that must be later freed
 * by the caller. The data_len parameter must be equal or less than the length
 * of the remaining data in the information elemnet.
 *
 * @param ie the information element.
 * @param data_len the number of bytes that must be read from the IE.
 * @out data returns a byte array.
 * @returns OK on success
 *          ACP_MSG_ERR_INCOMPLETE if IE has less remaining bytes than the
 *          number of bytes requested by the data_len parameter.
 *          ACP_MSG_ERR_NO_MEM if there's not enough memory to store the string.
 * @pre buff != NULL
 * @pre data != NULL
 * @internal
 */
extern e_ret acp_ie_read_bin_payload(acp_ie *ie, u8** data, u32 data_len);

/*
 * Reads an ISO-8859-1 string from the given IE.
 *
 * The function allocates memory for the string that must be later freed by
 * the caller.
 *
 * @param ie the information element.
 * @out data returns an NULL terminated string.
 * @returns OK on success
 *          ACP_MSG_ERR_NO_MEM if there's not enough memory to store the string.
 * @pre ie != NULL
 * @pre data != NULL
 * @pre ie->id == ACP_IE_ISO_8859_1 || ie->id == ACP_IE_EXT_ISO_8859_1
 * @internal
 */
extern e_ret acp_ie_read_iso_8859_1_payload(acp_ie *ie, ascii** data);

/*
 * Reads an BCD packed decimal from the given IE.
 *
 * The function allocates memory for the string that must be later freed by
 * the caller.
 *
 * @param ie the information element.
 * @out data returns a NULL terminated string of decimal digits.
 * @returns OK on success
 *          ACP_MSG_ERR_NO_MEM if there's not enough memory to store the string.
 * @pre buff != NULL
 * @pre data != NULL
 * @pre ie->id == ACP_IE_PACKED_DEC || ie->id == ACP_IE_EXT_PACKED_DEC
 * @internal
 */
extern e_ret acp_ie_read_bcd_payload(acp_ie *ie, ascii **data);

extern u32 acp_ie_get_any_len(acp_ie_any *ie);
extern e_ret acp_ie_read_any(e_buff *buff, acp_ie_any *ie);
extern e_ret acp_ie_write_any(e_buff *buff, acp_ie_any *ie);
extern void acp_ie_free_any(acp_ie_any *ie);

#endif

#ifdef __cplusplus
}
#endif

#endif
