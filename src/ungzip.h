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
 * ungzip.h - Minimal gzip decompressor
 */

#ifndef _UNGZIP_H_
#define _UNGZIP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ungzip - decompress a gzip-compressed buffer (RFC 1952 / RFC 1951 DEFLATE).
 *
 * src    - pointer to compressed data
 * srclen - byte length of compressed data
 * dst    - output buffer, or NULL to query the uncompressed size
 * dstlen - size of output buffer in bytes (0 to query size)
 *
 * Returns the number of bytes written to dst on success, or -1 on error.
 * When dst is NULL or dstlen is 0 the function reads the ISIZE field from
 * the gzip trailer and returns that value without decompressing.
 */
long ungzip(const uint8_t *src, long srclen, uint8_t *dst, long dstlen);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UNGZIP_H_ */
