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
 * ACP License verification.
 * @date 09/01/2009 04:10:05 PM
 * @file acp_license.c
 * @author Edantech
 * @internal
 */
/* Contributors:
 * Santiago Aguiar, santiago.aguiar@edantech.com
 */

#include "acp245_config.h"

#include "e_util.h"
#include "e_log.h"
#include "e_mem.h"

#include "acp_license.h"
#include "sha256.h"
#include "hmac_sha256.h"

#include <stdio.h>

static e_ret _get_line(FILE* f, ascii* buf, size_t n, size_t *readed) {
    size_t r = 0;
    int c = 0;

    e_assert( f != NULL );
    e_assert( buf != NULL );
    e_assert( n > 1 );

    while( r < n && ((c = fgetc(f)) != EOF)) {
        buf[r++] = (ascii)c;
        if (c == '\n') {
            break;
        }
    }

    if (r == n) {
        return ERROR;
    } else if( c == EOF ) {
        return ERROR;
    }
    buf[r] = '\0';
    *readed = r;
    return OK;
}

#define _KEY_SIZE   (SHA256_HASH_SIZE)
#define _MAX_TEXT   (4096u)
#define _MAX_LINE   (1024u)
#define _MAX_SIG    _MAX_LINE

#include "acp_license_secret.c.inc"

static bool _verified = FALSE;

e_ret acp_license_verify(const ascii *license_filename) {
    int i;
    u8 secret[] = _SECRET;
    u8 secret_key[_SECRET_SIZE];
    u8 hmac[SHA256_HASH_SIZE];
    ascii key[(_KEY_SIZE * 2) + 1];
    u8 keybin[_KEY_SIZE];
    ascii sig[_MAX_SIG];
    ascii line[_MAX_LINE];
    bool got_text = FALSE;
    e_ret rc;
    u8 text[_MAX_TEXT];
    u8 *text_pos = text;
    u8 *text_lim = text + _MAX_TEXT;
    FILE *license_file;

    license_file = fopen(license_filename, "r");
    if (!license_file) {
        E_DBG("License file not found");
        rc = ACP_LICENSE_NO_LICENSE;
        goto exit;
    }

    key[0] = '\0';
    sig[0] = '\0';
    while (!key[0] || !sig[0] || !got_text) {
        size_t r;
        rc = _get_line(license_file, line, _MAX_LINE - 1, &r);

        if (rc) {
            rc = ACP_LICENSE_INVALID_FORMAT;
            goto exit;
        }

        if (!got_text) {
            if (strstr(line, "--") == line) {
                E_DBG("Reading text");
                got_text = TRUE;
            } else if (text_pos + r < text_lim) {
                /* FIXME: assumes sizeof(u8) == sizeof(char), is that OK? */
                e_mem_cpy(text_pos, line, r);
                text_pos += r;
            } else {
                E_DBG("License too too long");
                rc = ACP_LICENSE_INVALID_FORMAT;
            }
        } else if (strstr(line, "Key: ") == line) {
            /* key must have at least one char */
            if ((r > 6) && ((r - 6) <=  _KEY_SIZE * 2)) {
                /* chop the 'Key: ' and the '\n' */
                strncat(key, line + 5, r - 6);
                E_DBG("Readed key");
                if(e_util_to_bin(keybin, key, _KEY_SIZE)) {
                    E_DBG("Key is not an hex string");
                    rc = ACP_LICENSE_INVALID_FORMAT;
                    goto exit;
                }
            }
        } else if (strstr(line, "Signature: ") == line) {
            /* signature must have at least one char */
            if ((r > 12) && ((r - 12) <=  _MAX_SIG)) {
                /* chop the 'Signature: ' and the '\n' */
                strncat(sig, line + 11, r - 12);
                E_DBG("Readed signature");
            }
        }
    }

    E_DBG("Key is: %s", key);
    E_DBG("Sig is: %s", sig);

    /* scramble secret_key a little ... */
    for (i = 0; i < _SECRET_SIZE; i++) {
        secret_key[i] = secret[i] ^ secret[secret[i] % _SECRET_SIZE];
    }

    hmac_sha256(secret_key, _SECRET_SIZE, text, (u16)(text_pos-text), hmac, SHA256_HASH_SIZE);
    if (e_mem_cmp(hmac, keybin, _KEY_SIZE) != 0) {
        rc = ACP_LICENSE_INVALID;
        goto exit;
    }

    _verified = TRUE;

    rc = ACP_LICENSE_VALID;
exit:
    if (license_file) {
        fclose(license_file);
    }
    return rc;
}

bool acp_license_verified(void) {
    return _verified;
}
