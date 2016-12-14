/**
  Copyright © 2016 Odzhan. All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. The name of the author may not be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY AUTHORS "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#include "belt.h"

// 32-bit type
typedef union {
    uint8_t  b[4];
    uint32_t w;
} w32_t;

// 128-bit type
typedef struct {
    union {
      uint8_t  v8[16];
      uint32_t w[4];
      struct {
        uint32_t a, b, c, d;
      };
    };
} w128_t;

// 256-bit type
typedef struct {
    union {
      uint8_t  v8[32];
      uint32_t w[8];
      struct {
        uint32_t a, b, c, d;
        uint32_t e, f, g, h;
      };
    };
} w256_t;

// sbox
const uint8_t H[256] =
{ 0xB1, 0x94, 0xBA, 0xC8, 0x0A, 0x08, 0xF5, 0x3B,
  0x36, 0x6D, 0x00, 0x8E, 0x58, 0x4A, 0x5D, 0xE4,
  0x85, 0x04, 0xFA, 0x9D, 0x1B, 0xB6, 0xC7, 0xAC,
  0x25, 0x2E, 0x72, 0xC2, 0x02, 0xFD, 0xCE, 0x0D,
  0x5B, 0xE3, 0xD6, 0x12, 0x17, 0xB9, 0x61, 0x81,
  0xFE, 0x67, 0x86, 0xAD, 0x71, 0x6B, 0x89, 0x0B,
  0x5C, 0xB0, 0xC0, 0xFF, 0x33, 0xC3, 0x56, 0xB8,
  0x35, 0xC4, 0x05, 0xAE, 0xD8, 0xE0, 0x7F, 0x99,
  0xE1, 0x2B, 0xDC, 0x1A, 0xE2, 0x82, 0x57, 0xEC,
  0x70, 0x3F, 0xCC, 0xF0, 0x95, 0xEE, 0x8D, 0xF1,
  0xC1, 0xAB, 0x76, 0x38, 0x9F, 0xE6, 0x78, 0xCA,
  0xF7, 0xC6, 0xF8, 0x60, 0xD5, 0xBB, 0x9C, 0x4F,
  0xF3, 0x3C, 0x65, 0x7B, 0x63, 0x7C, 0x30, 0x6A,
  0xDD, 0x4E, 0xA7, 0x79, 0x9E, 0xB2, 0x3D, 0x31,
  0x3E, 0x98, 0xB5, 0x6E, 0x27, 0xD3, 0xBC, 0xCF,
  0x59, 0x1E, 0x18, 0x1F, 0x4C, 0x5A, 0xB7, 0x93,
  0xE9, 0xDE, 0xE7, 0x2C, 0x8F, 0x0C, 0x0F, 0xA6,
  0x2D, 0xDB, 0x49, 0xF4, 0x6F, 0x73, 0x96, 0x47,
  0x06, 0x07, 0x53, 0x16, 0xED, 0x24, 0x7A, 0x37,
  0x39, 0xCB, 0xA3, 0x83, 0x03, 0xA9, 0x8B, 0xF6,
  0x92, 0xBD, 0x9B, 0x1C, 0xE5, 0xD1, 0x41, 0x01,
  0x54, 0x45, 0xFB, 0xC9, 0x5E, 0x4D, 0x0E, 0xF2,
  0x68, 0x20, 0x80, 0xAA, 0x22, 0x7D, 0x64, 0x2F,
  0x26, 0x87, 0xF9, 0x34, 0x90, 0x40, 0x55, 0x11,
  0xBE, 0x32, 0x97, 0x13, 0x43, 0xFC, 0x9A, 0x48,
  0xA0, 0x2A, 0x88, 0x5F, 0x19, 0x4B, 0x09, 0xA1,
  0x7E, 0xCD, 0xA4, 0xD0, 0x15, 0x44, 0xAF, 0x8C,
  0xA5, 0x84, 0x50, 0xBF, 0x66, 0xD2, 0xE8, 0x8A,
  0xA2, 0xD7, 0x46, 0x52, 0x42, 0xA8, 0xDF, 0xB3,
  0x69, 0x74, 0xC5, 0x51, 0xEB, 0x23, 0x29, 0x21,
  0xD4, 0xEF, 0xD9, 0xB4, 0x3A, 0x62, 0x28, 0x75,
  0x91, 0x14, 0x10, 0xEA, 0x77, 0x6C, 0xDA, 0x1D };

// substitute 32-bits
uint32_t G(uint32_t x, w256_t *key, int idx, int r) {
    int   i;
    w32_t u;

    u.w = key->w[idx & 7] + x;

    for (i=0; i<4; i++) {
      u.b[i] = H[u.b[i]];
    }
    return ROTL32(u.w, r);
}

// perform encryption and decryption based on enc parameter
void belt_encrypt(void *blk, const void *ks, int enc)
{
    w256_t   v; 
    w256_t   key;
    uint32_t i, j, t, e;
    uint32_t *x=(uint32_t*)blk;

    // load 256-bit key into local space
    memcpy (key.v8, (uint8_t*)ks, 32);

    // if decryption, rotate key 128-bits
    if (enc==BELT_DECRYPT) {
      for (i=0; i<4; i++) {
        XCHG (key.w[i], key.w[7-i], t);
      }
    }
    
    // load 128-bit data into local space
    memcpy (v.v8, (uint8_t*)blk, 16);

    // apply 8 rounds
    for (i=0, j=0; i<8; i++, j += 7)
    {
      v.b ^= G(v.a,       &key, j+0, 5);
      v.c ^= G(v.d,       &key, j+1,21);
      v.a -= G(v.b,       &key, j+2,13);
      e    = G(v.b + v.c, &key, j+3,21);
      t    = i + 1;

      if (enc==BELT_ENCRYPT) 
          goto b_l3;
      
      t = (7 - i) + 1;
b_l3:
      e   ^= t;
      v.b += e;
      v.c -= e;
      v.d += G(v.c,     &key, j+4,13);
      v.b ^= G(v.a,     &key, j+5,21);
      v.c ^= G(v.d,     &key, j+6, 5);

      XCHG(v.a, v.b, t);
      XCHG(v.c, v.d, t);
      XCHG(v.b, v.c, t);

      if (enc==BELT_ENCRYPT)
          continue;
      
      // swap for decryption
      XCHG(v.b, v.c, t);
      XCHG(v.a, v.d, t);
    }
    // save data for encryption
    x[0] = v.b; x[1] = v.d;
    x[2] = v.a; x[3] = v.c;

    if (enc == BELT_ENCRYPT)
        return;
      
    // save data for decryption
    x[0] = v.c; x[1] = v.a;
    x[2] = v.d; x[3] = v.b;
}
