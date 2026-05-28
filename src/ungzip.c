/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2025 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * ungzip.c - Minimal gzip decompressor
 *
 * The DEFLATE decoder is based on puff.c by Mark Adler (public domain).
 * Gzip header / trailer parsing follows RFC 1952.
 *
 * REFERENCES:
 * https://www.ietf.org/rfc/rfc1952.txt  (gzip format)
 * https://www.ietf.org/rfc/rfc1951.txt  (DEFLATE compressed data format)
 * https://github.com/madler/zlib/blob/master/contrib/puff/puff.c
 */

#include <string.h>
#include "ungzip.h"

/* ---- DEFLATE decoder (based on puff.c by Mark Adler, public domain) ----- */

#define MAXBITS    15               /* maximum bits in a Huffman code        */
#define MAXLCODES 286               /* maximum literal/length codes          */
#define MAXDCODES  30               /* maximum distance codes                */
#define MAXCODES  (MAXLCODES + MAXDCODES)   /* max codes in a code-len code  */
#define FIXLCODES 288               /* number of fixed literal/length codes  */

struct huffman {
    short count[MAXBITS + 1];       /* number of symbols of each code length */
    short symbol[MAXCODES];         /* canonically ordered symbols           */
};

struct state {
    /* input */
    const uint8_t *in;
    long           inlen;
    long           incnt;
    int            bitbuf;
    int            bitcnt;
    /* output */
    uint8_t       *out;
    long           outlen;
    long           outcnt;
};

/* Fetch 'need' bits from the input stream; returns the value or -1 on
 * underflow. Bits are consumed LSB-first (DEFLATE convention). */
static int bits(struct state *s, int need)
{
    long val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->incnt == s->inlen) return -1;    /* out of input */
        val |= (long)(s->in[s->incnt++]) << s->bitcnt;
        s->bitcnt += 8;
    }
    s->bitbuf  = (int)(val >> need);
    s->bitcnt -= need;
    return (int)(val & ((1L << need) - 1));
}

/* Decode one symbol from the stream using Huffman table h. */
static int decode(struct state *s, const struct huffman *h)
{
    int len, code, first, count, index;

    code = first = index = 0;
    for (len = 1; len <= MAXBITS; len++) {
        int b = bits(s, 1);
        if (b < 0) return -10;
        code  |= b;
        count  = h->count[len];
        if (code - count < first)               /* length len: return symbol */
            return h->symbol[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code  <<= 1;
    }
    return -10;                                 /* no code found */
}

/* Build a canonical Huffman table from an array of code lengths.
 * Returns 0 (complete), > 0 (incomplete), or < 0 (oversubscribed). */
static int construct(struct huffman *h, const short *length, int n)
{
    int   sym, len, left;
    short offs[MAXBITS + 1];

    for (len = 0; len <= MAXBITS; len++) h->count[len] = 0;
    for (sym  = 0; sym  < n;       sym++) h->count[length[sym]]++;
    if (h->count[0] == n) return 0;            /* only zero-length codes     */

    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left  -= h->count[len];
        if (left < 0) return left;             /* oversubscribed             */
    }

    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->count[len];
    for (sym = 0; sym < n; sym++)
        if (length[sym] != 0)
            h->symbol[offs[length[sym]]++] = sym;

    return left;                               /* left > 0 means incomplete  */
}

/* Decode a non-compressed (stored) block. */
static int stored(struct state *s)
{
    unsigned len, cmp;

    s->bitbuf = 0; s->bitcnt = 0;              /* flush any partial byte     */
    if (s->incnt + 4 > s->inlen) return -2;
    len  =         s->in[s->incnt++];
    len |= (unsigned)s->in[s->incnt++] << 8;
    cmp  =         s->in[s->incnt++];
    cmp |= (unsigned)s->in[s->incnt++] << 8;
    if (len != (~cmp & 0xffff)) return -2;     /* length check               */
    if (s->incnt  + (long)len > s->inlen)  return -2;
    if (s->outcnt + (long)len > s->outlen) return -1; /* output overflow     */
    memcpy(s->out + s->outcnt, s->in + s->incnt, len);
    s->incnt  += len;
    s->outcnt += len;
    return 0;
}

/* Decode literal/length symbols and back-references using Huffman tables. */
static int codes(struct state *s,
                 const struct huffman *lencode,
                 const struct huffman *distcode)
{
    /* Size base and extra bits for length codes 257..285 */
    static const short lens[29] = {
        3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,
        35,43,51,59,67,83,99,115,131,163,195,227,258 };
    static const short lext[29] = {
        0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,
        3,3,3,3,4,4,4,4,5,5,5,5,0 };
    /* Distance base and extra bits for distance codes 0..29 */
    static const short dists[30] = {
        1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
        257,385,513,769,1025,1537,2049,3073,4097,6145,
        8193,12289,16385,24577 };
    static const short dext[30] = {
        0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,
        7,7,8,8,9,9,10,10,11,11,12,12,13,13 };

    int symbol;
    do {
        symbol = decode(s, lencode);
        if (symbol < 0) return symbol;

        if (symbol < 256) {
            /* literal byte */
            if (s->outcnt == s->outlen) return -1;
            s->out[s->outcnt++] = (uint8_t)symbol;
        } else if (symbol > 256) {
            /* length / distance back-reference */
            int eb, len;
            unsigned dist;

            symbol -= 257;
            if (symbol >= 29) return -10;       /* invalid code               */
            eb  = bits(s, lext[symbol]);
            if (eb < 0) return eb;
            len = lens[symbol] + eb;

            symbol = decode(s, distcode);
            if (symbol < 0) return symbol;
            eb   = bits(s, dext[symbol]);
            if (eb < 0) return eb;
            dist = (unsigned)dists[symbol] + (unsigned)eb;

            if ((long)dist > s->outcnt) return -10; /* distance too far back  */
            while (len--) {
                if (s->outcnt >= s->outlen) return -1;
                s->out[s->outcnt] = s->out[s->outcnt - dist];
                s->outcnt++;
            }
        }
    } while (symbol != 256);                   /* 256 = end-of-block         */
    return 0;
}

/* Decode a block using fixed (pre-defined) Huffman codes (BTYPE = 01). */
static int fixed(struct state *s)
{
    static int           once = 0;
    static struct huffman lenc, distc;

    if (!once) {
        short lengths[FIXLCODES];
        int   sym;
        for (sym =   0; sym < 144;       sym++) lengths[sym] = 8;
        for (        ; sym < 256;        sym++) lengths[sym] = 9;
        for (        ; sym < 280;        sym++) lengths[sym] = 7;
        for (        ; sym < FIXLCODES;  sym++) lengths[sym] = 8;
        construct(&lenc, lengths, FIXLCODES);
        for (sym = 0; sym < MAXDCODES; sym++) lengths[sym] = 5;
        construct(&distc, lengths, MAXDCODES);
        once = 1;
    }
    return codes(s, &lenc, &distc);
}

/* Decode a block using dynamic Huffman codes (BTYPE = 10). */
static int dynamic(struct state *s)
{
    /* Permutation order for code-length code lengths */
    static const short order[19] = {
        16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };

    int    nlen, ndist, ncode, index;
    short  lengths[MAXCODES];
    struct huffman lenc, distc;

    int hlit  = bits(s, 5); if (hlit  < 0) return hlit;
    int hdist = bits(s, 5); if (hdist < 0) return hdist;
    int hclen = bits(s, 4); if (hclen < 0) return hclen;

    nlen  = hlit  + 257;
    ndist = hdist + 1;
    ncode = hclen + 4;

    if (nlen > MAXLCODES || ndist > MAXDCODES) return -3;

    /* Read code-length code lengths and build the temporary tree */
    for (index = 0; index < 19;    index++) lengths[order[index]] = 0;
    for (index = 0; index < ncode; index++) {
        int v = bits(s, 3); if (v < 0) return v;
        lengths[order[index]] = (short)v;
    }

    {
        struct huffman clenc;
        int err = construct(&clenc, lengths, 19);
        if (err < 0) return -4;

        /* Decode the literal/length and distance code lengths */
        index = 0;
        while (index < nlen + ndist) {
            int sym = decode(s, &clenc);
            if (sym < 0) return sym;
            if (sym < 16) {
                lengths[index++] = (short)sym;
            } else {
                short rep = 0;
                int   cnt;
                if (sym == 16) {
                    if (!index) return -5;
                    rep = lengths[index - 1];
                    int x = bits(s, 2); if (x < 0) return x;
                    cnt = 3 + x;
                } else if (sym == 17) {
                    int x = bits(s, 3); if (x < 0) return x;
                    cnt = 3 + x;
                } else {            /* sym == 18 */
                    int x = bits(s, 7); if (x < 0) return x;
                    cnt = 11 + x;
                }
                if (index + cnt > nlen + ndist) return -6;
                while (cnt--) lengths[index++] = rep;
            }
        }
    }

    if (lengths[256] == 0) return -9;  /* end-of-block code must be present */

    {
        int err;
        err = construct(&lenc,  lengths,        nlen);
        if (err < 0 || (err > 0 && nlen  - lenc.count[0]  != 1)) return -7;
        err = construct(&distc, lengths + nlen, ndist);
        if (err < 0 || (err > 0 && ndist - distc.count[0] != 1)) return -8;
    }

    return codes(s, &lenc, &distc);
}

/* Top-level DEFLATE decompressor.
 * On return, *destlen holds the number of bytes actually written. */
static int puff(uint8_t *dest, long *destlen,
                const uint8_t *source, long sourcelen)
{
    struct state s;
    int err = 0, last, type;

    s.out    = dest;
    s.outlen = *destlen;
    s.outcnt = 0;
    s.in     = source;
    s.inlen  = sourcelen;
    s.incnt  = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    do {
        last = bits(&s, 1); if (last < 0) { err = last; break; }
        type = bits(&s, 2); if (type < 0) { err = type; break; }
        err  = (type == 0 ? stored(&s)  :
                type == 1 ? fixed(&s)   :
                type == 2 ? dynamic(&s) : -1);
        if (err) break;
    } while (!last);

    *destlen = s.outcnt;
    return err;
}

/* ---- gzip header parser (RFC 1952) -------------------------------------- */

#define GZ_FHCRC    0x02
#define GZ_FEXTRA   0x04
#define GZ_FNAME    0x08
#define GZ_FCOMMENT 0x10

long ungzip(const uint8_t *src, long srclen, uint8_t *dst, long dstlen)
{
    long    pos;
    uint8_t flags;
    long    isize, complen;

    if (srclen < 18)                   return -1; /* too short for gzip      */
    if (src[0] != 0x1F || src[1] != 0x8B) return -1; /* magic mismatch      */
    if (src[2] != 8)                   return -1; /* method must be DEFLATE  */

    flags = src[3];
    pos   = 10;                        /* skip fixed 10-byte gzip header     */

    if (flags & GZ_FEXTRA) {
        unsigned xlen;
        if (pos + 2 > srclen) return -1;
        xlen  = src[pos] | ((unsigned)src[pos + 1] << 8);
        pos  += 2 + xlen;
    }
    if (flags & GZ_FNAME) {
        while (pos < srclen && src[pos]) pos++;
        if (pos >= srclen) return -1;
        pos++;                         /* skip null terminator               */
    }
    if (flags & GZ_FCOMMENT) {
        while (pos < srclen && src[pos]) pos++;
        if (pos >= srclen) return -1;
        pos++;
    }
    if (flags & GZ_FHCRC)
        pos += 2;

    if (pos >= srclen - 8) return -1;  /* need at least CRC32 + ISIZE        */

    /* ISIZE: original (uncompressed) size stored in the last 4 bytes */
    isize = (long) src[srclen - 4]
          | ((long)src[srclen - 3] << 8)
          | ((long)src[srclen - 2] << 16)
          | ((long)src[srclen - 1] << 24);

    if (!dst || dstlen == 0)
        return isize;                  /* size query — no decompression      */

    complen = srclen - pos - 8;        /* compressed data: skip CRC32+ISIZE  */
    if (complen <= 0) return -1;

    if (puff(dst, &dstlen, src + pos, complen) != 0)
        return -1;

    return dstlen;
}
