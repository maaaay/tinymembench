/*
 * Copyright © 2011 Siarhei Siamashka <siarhei.siamashka@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include "util.h"

void aligned_block_copy_noprefetch(int64_t * __restrict dst,
                                   int64_t * __restrict src,
                                   int                  size)
{
    while ((size -= 32) >= 0)
    {
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
    }
}

void aligned_block_copy_backwards_noprefetch(int64_t * __restrict dst,
                                             int64_t * __restrict src,
                                             int                  size)
{
    src += size / 8 - 1;
    dst += size / 8 - 1;
    while ((size -= 32) >= 0)
    {
        *dst-- = *src--;
        *dst-- = *src--;
        *dst-- = *src--;
        *dst-- = *src--;
    }
}

void aligned_block_copy(int64_t * __restrict dst,
                        int64_t * __restrict src,
                        int                  size)
{
    while ((size -= 32) >= 0)
    {
        __builtin_prefetch(src + 32, 0, 0);
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
    }
}

double gettime(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (double)((int64_t)tv.tv_sec * 1000000 + tv.tv_usec) / 1000000.;
}

double fmin(double a, double b)
{
    return a < b ? a : b;
}

#define ALIGN_PADDING    0x100000
#define CACHE_LINE_SIZE  128

static char *align_up(char *ptr, int align)
{
    return (char *)(((uintptr_t)ptr + align - 1) & ~(uintptr_t)(align - 1));
}

void *alloc_four_nonaliased_buffers(char **buf1, int size1,
                                    char **buf2, int size2,
                                    char **buf3, int size3,
                                    char **buf4, int size4)
{
    int antialias_pattern_mask = (ALIGN_PADDING - 1) & ~(CACHE_LINE_SIZE - 1);
    char *buf, *ptr;

    if (!buf1 || size1 < 0)
        size1 = 0;
    if (!buf2 || size2 < 0)
        size2 = 0;
    if (!buf3 || size3 < 0)
        size3 = 0;
    if (!buf4 || size4 < 0)
        size4 = 0;

    ptr = buf = malloc(size1 + size2 + size3 + size4 + 9 * ALIGN_PADDING);
    memset(buf, 0xCC, size1 + size2 + size3 + size4 + 9 * ALIGN_PADDING);

    ptr = align_up(ptr, ALIGN_PADDING);
    if (buf1)
    {
        *buf1 = ptr + (0xAAAAAAAA & antialias_pattern_mask);
        ptr = align_up(*buf1 + size1, ALIGN_PADDING);
    }
    if (buf2)
    {
        *buf2 = ptr + (0x55555555 & antialias_pattern_mask);
        ptr = align_up(*buf2 + size2, ALIGN_PADDING);
    }
    if (buf3)
    {
        *buf3 = ptr + (0xCCCCCCCC & antialias_pattern_mask);
        ptr = align_up(*buf3 + size3, ALIGN_PADDING);
    }
    if (buf4)
    {
        *buf3 = ptr + (0x33333333 & antialias_pattern_mask);
        ptr = align_up(*buf4 + size4, ALIGN_PADDING);
    }

    return buf;
}
