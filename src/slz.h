/*
 * Copyright (C) 2013-2015 Willy Tarreau <w@1wt.eu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _SLZ_H
#define _SLZ_H

#include <stdint.h>

/* We have two macros UNALIGNED_LE_OK and UNALIGNED_FASTER. The latter indicates
 * that using unaligned data is faster than a simple shift. On x86 32-bit at
 * least it is not the case as the per-byte access is 30% faster. A core2-duo on
 * x86_64 is 7% faster to read one byte + shifting by 8 than to read one word,
 * but a core i5 is 7% faster doing the unaligned read, so we privilege more
 * recent implementations here.
 */
#if defined(__x86_64__)
#define UNALIGNED_LE_OK
#define UNALIGNED_FASTER
#define USE_64BIT_QUEUE
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#define UNALIGNED_LE_OK
//#define UNALIGNED_FASTER
#elif defined(__ARMEL__) && defined(__ARM_ARCH_7A__)
#define UNALIGNED_LE_OK
#define UNALIGNED_FASTER
#elif defined(__ARM_ARCH_8A) || defined(__ARM_FEATURE_UNALIGNED)
#define UNALIGNED_LE_OK
#define UNALIGNED_FASTER
#endif

/* Log2 of the size of the hash table used for the references table. */
#define HASH_BITS 13

enum slz_state {
	SLZ_ST_INIT,  /* stream initialized */
	SLZ_ST_EOB,   /* header or end of block already sent */
	SLZ_ST_FIXED, /* inside a fixed huffman sequence */
	SLZ_ST_LAST,  /* last block, BFINAL sent */
	SLZ_ST_DONE,  /* BFINAL+EOB sent BFINAL */
	SLZ_ST_END    /* end sent (BFINAL, EOB, CRC + len) */
};

enum {
	SLZ_FMT_GZIP,    /* RFC1952: gzip envelope and crc32 for CRC */
	SLZ_FMT_ZLIB,    /* RFC1950: zlib envelope and adler-32 for CRC */
	SLZ_FMT_DEFLATE, /* RFC1951: raw deflate, and no crc */
};

#pragma pack(push,4)
struct slz_stream {
#ifdef USE_64BIT_QUEUE
	uint64_t queue; /* last pending bits, LSB first */
#else
	uint32_t queue; /* last pending bits, LSB first */
#endif
	uint32_t qbits; /* number of bits in queue, < 8 on 32-bit, < 32 on 64-bit */
	unsigned char *outbuf; /* set by encode() */
	uint16_t state; /* one of slz_state */
	uint8_t level; /* 0 = no compression, 1 = compression */
	uint8_t format; /* SLZ_FMT_* */
	//uint8_t unused1; /* unused for now */
	uint32_t crc32;
	uint32_t ilen;
};
#pragma pack(pop)

/* Functions specific to rfc1951 (deflate) */
void slz_prepare_dist_table(); /* obsolete, not needed anymore */
long slz_rfc1951_encode(struct slz_stream *strm, unsigned char *out, const unsigned char *in, long ilen, int more);
int slz_rfc1951_init(struct slz_stream *strm, int level);
int slz_rfc1951_finish(struct slz_stream *strm, unsigned char *buf);

/* Functions specific to rfc1952 (gzip) */
void slz_make_crc_table(void); /* obsolete, not needed anymore */
uint32_t slz_crc32_by1(uint32_t crc, const unsigned char *buf, int len);
uint32_t slz_crc32_by4(uint32_t crc, const unsigned char *buf, int len);
long slz_rfc1952_encode(struct slz_stream *strm, unsigned char *out, const unsigned char *in, long ilen, int more);
int slz_rfc1952_send_header(struct slz_stream *strm, unsigned char *buf);
int slz_rfc1952_init(struct slz_stream *strm, int level);
int slz_rfc1952_finish(struct slz_stream *strm, unsigned char *buf);

/* Functions specific to rfc1950 (zlib) */
uint32_t slz_adler32_by1(uint32_t crc, const unsigned char *buf, int len);
uint32_t slz_adler32_block(uint32_t crc, const unsigned char *buf, long len);
long slz_rfc1950_encode(struct slz_stream *strm, unsigned char *out, const unsigned char *in, long ilen, int more);
int slz_rfc1950_send_header(struct slz_stream *strm, unsigned char *buf);
int slz_rfc1950_init(struct slz_stream *strm, int level);
int slz_rfc1950_finish(struct slz_stream *strm, unsigned char *buf);

int slz_init(struct slz_stream *strm, int level, int format);
long slz_encode(struct slz_stream *strm, void *out, const void *in, long ilen, int more);
int slz_finish(struct slz_stream *strm, void *buf);

#endif
