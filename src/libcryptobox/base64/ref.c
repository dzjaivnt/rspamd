/*-
Copyright (c) 2013-2015, Alfred Klomp
Copyright (c) 2016, Vsevolod Stakhov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

const uint8_t
base64_table_dec[] =
{
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
	 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255,
	255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
	 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
	255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
	 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};

#define INNER_LOOP_64 do { \
	while (inlen >= 13) { \
		uint64_t str, res, dec; \
		str = *(uint64_t *)c; \
		str = GUINT64_TO_BE(str); \
		if ((dec = base64_table_dec[str >> 56]) > 63) { \
			break; \
		} \
		res = dec << 58; \
		if ((dec = base64_table_dec[(str >> 48) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 52; \
		if ((dec = base64_table_dec[(str >> 40) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 46; \
		if ((dec = base64_table_dec[(str >> 32) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 40; \
		if ((dec = base64_table_dec[(str >> 24) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 34; \
		if ((dec = base64_table_dec[(str >> 16) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 28; \
		if ((dec = base64_table_dec[(str >> 8) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 22; \
		if ((dec = base64_table_dec[str & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 16; \
		res = GUINT64_FROM_BE(res); \
		*(uint64_t *)o = res; \
		c += 8; \
		o += 6; \
		outl += 6; \
		inlen -= 8; \
	} \
} while (0)

#define INNER_LOOP_32 do { \
	while (inlen >= 8) { \
		uint32_t str, res, dec; \
		str = *(uint32_t *)c; \
		str = GUINT32_TO_BE(str); \
		if ((dec = base64_table_dec[str >> 24]) > 63) { \
			break; \
		} \
		res = dec << 26; \
		if ((dec = base64_table_dec[(str >> 16) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 20; \
		if ((dec = base64_table_dec[(str >> 8) & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 14; \
		if ((dec = base64_table_dec[str & 0xFF]) > 63) { \
			break; \
		} \
		res |= dec << 8; \
		res = GUINT32_FROM_BE(res); \
		*(uint32_t *)o = res; \
		c += 4; \
		o += 3; \
		outl += 3; \
		inlen -= 4; \
	} \
} while (0)


int
base64_decode_ref (const char *in, size_t inlen,
		unsigned char *out, size_t *outlen)
{
	ssize_t ret = 0;
	const uint8_t *c = (const uint8_t *)in;
	uint8_t *o = (uint8_t *)out;
	uint8_t q, carry;
	size_t outl = 0;
	size_t leftover = 0;

	switch (leftover) {
		for (;;) {
		case 0:
#if defined(__LP64__)
			INNER_LOOP_64;
#else
			INNER_LOOP_32;
#endif

			if (inlen-- == 0) {
				ret = 1;
				break;
			}
			if ((q = base64_table_dec[*c++]) >= 254) {

				break;
			}
			carry = q << 2;
			leftover++;

		case 1:
			if (inlen-- == 0) {
				ret = 1;
				break;
			}
			if ((q = base64_table_dec[*c++]) >= 254) {
				return (-1);
			}
			*o++ = carry | (q >> 4);
			carry = q << 4;
			leftover++;
			outl++;

		case 2:
			if (inlen-- == 0) {
				ret = 1;
				break;
			}
			if ((q = base64_table_dec[*c++]) >= 254) {
				leftover++;

				if (q == 254) {
					if (inlen-- != 0) {
						leftover = 0;
						q = base64_table_dec[*c++];
						ret = ((q == 254) && (inlen == 0)) ? 1 : 0;
						break;
					}
					else {
						ret = 1;
						break;
					}
				}
				/* If we get here, there was an error: */
				break;
			}
			*o++ = carry | (q >> 2);
			carry = q << 6;
			leftover++;
			outl++;

		case 3:
			if (inlen-- == 0) {
				ret = 1;
				break;
			}
			if ((q = base64_table_dec[*c++]) >= 254) {
				leftover = 0;
				/*
				 * When q == 254, the input char is '='. Return 1 and EOF.
				 * When q == 255, the input char is invalid. Return 0 and EOF.
				 */
				ret = ((q == 254) && (inlen == 0)) ? 1 : 0;
				break;
			}

			*o++ = carry | q;
			carry = 0;
			leftover = 0;
			outl++;
		}
	}

	*outlen = outl;

	return ret;
}
