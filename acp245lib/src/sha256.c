/*-
 * Copyright (c) 2001-2003 Allan Saddi <allan@saddi.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ALLAN SADDI AND HIS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL ALLAN SADDI OR HIS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: sha256.c 680 2003-07-25 21:57:49Z asaddi $
 */
/*
 * Define WORDS_BIGENDIAN if compiling on a big-endian architecture.
 *
 * Define SHA256_TEST to test the implementation using the NIST's
 * sample messages. The output should be:
 *
 *   ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad
 *   248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 64ff2167 f6ecedd4 19db06c1
 *   cdc76e5c 9914fb92 81a1c7e2 84d73e67 f1809a48 a497200e 046d39cc c7112cd0
 */
#include "acp245_config.h"

#include "sha256.h"

#include "e_port.h"
#include "e_mem.h"

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#define Ch(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define Maj(x, y, z) (((x) & ((y) | (z))) | ((y) & (z)))
#define SIGMA0(x) (ROTR((x), 2) ^ ROTR((x), 13) ^ ROTR((x), 22))
#define SIGMA1(x) (ROTR((x), 6) ^ ROTR((x), 11) ^ ROTR((x), 25))
#define sigma0(x) (ROTR((x), 7) ^ ROTR((x), 18) ^ ((x) >> 3))
#define sigma1(x) (ROTR((x), 17) ^ ROTR((x), 19) ^ ((x) >> 10))

#define DO_ROUND() { \
  t1 = h + SIGMA1(e) + Ch(e, f, g) + *(Kp++) + *(W++); \
  t2 = SIGMA0(a) + Maj(a, b, c); \
  h = g; \
  g = f; \
  f = e; \
  e = d + t1; \
  d = c; \
  c = b; \
  b = a; \
  a = t1 + t2; \
}

static const u32 _K[64] = {
    0x428a2f98L, 0x71374491L, 0xb5c0fbcfL, 0xe9b5dba5L,
    0x3956c25bL, 0x59f111f1L, 0x923f82a4L, 0xab1c5ed5L,
    0xd807aa98L, 0x12835b01L, 0x243185beL, 0x550c7dc3L,
    0x72be5d74L, 0x80deb1feL, 0x9bdc06a7L, 0xc19bf174L,
    0xe49b69c1L, 0xefbe4786L, 0x0fc19dc6L, 0x240ca1ccL,
    0x2de92c6fL, 0x4a7484aaL, 0x5cb0a9dcL, 0x76f988daL,
    0x983e5152L, 0xa831c66dL, 0xb00327c8L, 0xbf597fc7L,
    0xc6e00bf3L, 0xd5a79147L, 0x06ca6351L, 0x14292967L,
    0x27b70a85L, 0x2e1b2138L, 0x4d2c6dfcL, 0x53380d13L,
    0x650a7354L, 0x766a0abbL, 0x81c2c92eL, 0x92722c85L,
    0xa2bfe8a1L, 0xa81a664bL, 0xc24b8b70L, 0xc76c51a3L,
    0xd192e819L, 0xd6990624L, 0xf40e3585L, 0x106aa070L,
    0x19a4c116L, 0x1e376c08L, 0x2748774cL, 0x34b0bcb5L,
    0x391c0cb3L, 0x4ed8aa4aL, 0x5b9cca4fL, 0x682e6ff3L,
    0x748f82eeL, 0x78a5636fL, 0x84c87814L, 0x8cc70208L,
    0x90befffaL, 0xa4506cebL, 0xbef9a3f7L, 0xc67178f2L
};

static const u8 _padding[64] = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void _burnStack(int size)
{
    char buf[128];

    e_mem_set (buf, 0, sizeof (buf));
    size -= sizeof (buf);
    if (size > 0) {
        _burnStack (size);
    }
}

static void _SHA256Guts(SHA256Context *sc, const u32 *cbuf)
{
    u32 buf[64];
    u32 *W, *W2, *W7, *W15, *W16;
    u32 a, b, c, d, e, f, g, h;
    u32 t1, t2;
    const u32 *Kp;
    int i;

    W = buf;

    for (i = 15; i >= 0; i--) {
        /* FIXME: Configure platform endianess */
        *W = ((*cbuf & 0xFF) << 24)
            | ((*cbuf & 0xFF00) << 8)
            | ((*cbuf & 0xFF0000) >> 8)
            | ((*cbuf & 0xFF000000) >> 24);
        W++;
        cbuf++;
    }

    W16 = &buf[0];
    W15 = &buf[1];
    W7 = &buf[9];
    W2 = &buf[14];

    for (i = 47; i >= 0; i--) {
        *(W++) = sigma1(*W2) + *(W7++) + sigma0(*W15) + *(W16++);
        W2++;
        W15++;
    }

    a = sc->hash[0];
    b = sc->hash[1];
    c = sc->hash[2];
    d = sc->hash[3];
    e = sc->hash[4];
    f = sc->hash[5];
    g = sc->hash[6];
    h = sc->hash[7];

    Kp = _K;
    W = buf;

    for (i = 63; i >= 0; i--) {
        DO_ROUND();
    }

    sc->hash[0] += a;
    sc->hash[1] += b;
    sc->hash[2] += c;
    sc->hash[3] += d;
    sc->hash[4] += e;
    sc->hash[5] += f;
    sc->hash[6] += g;
    sc->hash[7] += h;
}

void SHA256Init(SHA256Context *sc) {
    sc->totalLength = 0L;
    sc->hash[0] = 0x6a09e667L;
    sc->hash[1] = 0xbb67ae85L;
    sc->hash[2] = 0x3c6ef372L;
    sc->hash[3] = 0xa54ff53aL;
    sc->hash[4] = 0x510e527fL;
    sc->hash[5] = 0x9b05688cL;
    sc->hash[6] = 0x1f83d9abL;
    sc->hash[7] = 0x5be0cd19L;
    sc->bufferLength = 0L;
}

void SHA256Update(SHA256Context *sc, const void *vdata, u32 len) {
    const u8 *data = vdata;
    u32 bufferBytesLeft;
    u32 bytesToCopy;
    int needBurn = 0;

    while (len) {
        bufferBytesLeft = 64L - sc->bufferLength;

        bytesToCopy = bufferBytesLeft;
        if (bytesToCopy > len) {
            bytesToCopy = len;
        }

        e_mem_cpy (&sc->buffer.bytes[sc->bufferLength], data, bytesToCopy);

        sc->totalLength += bytesToCopy * 8L;

        sc->bufferLength += bytesToCopy;
        data += bytesToCopy;
        len -= bytesToCopy;

        if (sc->bufferLength == 64L) {
            _SHA256Guts (sc, sc->buffer.words);
            needBurn = 1;
            sc->bufferLength = 0L;
        }
    }

    if (needBurn) {
        _burnStack (sizeof (u32[74]) + sizeof (u32 *[6]) + sizeof (int));
    }
}

/*@-fixedformalarray@*/
void SHA256Final(SHA256Context *sc, u8 hash[SHA256_HASH_SIZE]) {
/*@+fixedformalarray@*/
    u32 bytesToPad;
    u8 lengthPad[8];
    int i;

    e_assert(hash != NULL);

    bytesToPad = 120L - sc->bufferLength;
    if (bytesToPad > 64L) {
        bytesToPad -= 64L;
    }

    e_mem_set(&lengthPad, 0, 8);
    lengthPad[0] = ((sc->totalLength>>24) & 0xFF);
    lengthPad[1] = ((sc->totalLength>>16) & 0xFF);
    lengthPad[2] = ((sc->totalLength>>8) & 0xFF);
    lengthPad[3] = (sc->totalLength & 0xFF);

    SHA256Update (sc, _padding, bytesToPad);
    SHA256Update (sc, &lengthPad[4], 4L);
    SHA256Update (sc, &lengthPad[0], 4L);

    for (i = 0; i < SHA256_HASH_WORDS; i++) {
        hash[0] = (u8) (0xFF & (sc->hash[i] >> 24));
        hash[1] = (u8) (0xFF & (sc->hash[i] >> 16));
        hash[2] = (u8) (0xFF & (sc->hash[i] >> 8));
        hash[3] = (u8) (0xFF & sc->hash[i]);
        hash += 4;
    }
}
