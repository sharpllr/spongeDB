/* zmalloc - total amount of allocated memory aware version of malloc()
 *
 * Copyright (c) 2009-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>

/* This function provide us access to the original libc free(). This is useful
 * for instance to free results obtained by backtrace_symbols(). We need
 * to define this function before including zmalloc.h that may shadow the
 * free implementation if we use jemalloc or another non standard allocator. */
void zlibc_free(void *ptr)
{
    free(ptr);
}

#include <string.h>
#include <pthread.h>
#include "mem/zmalloc.h"

#define PREFIX_SIZE (sizeof(size_t))

#if defined(HAVE_ATOMIC)    //Use gcc build-in atomic operations
#define update_zmalloc_stat_add(__n) __sync_add_and_fetch(&used_memory, (__n))
#define update_zmalloc_stat_sub(__n) __sync_sub_and_fetch(&used_memory, (__n))
#else                       //pthread mutex is inefficient
#define update_zmalloc_stat_add(__n) do { \
    pthread_mutex_lock(&used_memory_mutex); \
    used_memory += (__n); \
    pthread_mutex_unlock(&used_memory_mutex); \
} while(0)

#define update_zmalloc_stat_sub(__n) do { \
    pthread_mutex_lock(&used_memory_mutex); \
    used_memory -= (__n); \
    pthread_mutex_unlock(&used_memory_mutex); \
} while(0)

#endif

#define update_zmalloc_stat_alloc(__n) do { \
    size_t _n = (__n); \
    if (_n&(sizeof(long)-1)) _n += sizeof(long)-(_n&(sizeof(long)-1)); \
    if (zmalloc_thread_safe) { \
        update_zmalloc_stat_add(_n); \
    } else { \
        used_memory += _n; \
    } \
} while(0)

#define update_zmalloc_stat_free(__n) do { \
    size_t _n = (__n); \
    if (_n&(sizeof(long)-1)) _n += sizeof(long)-(_n&(sizeof(long)-1)); \
    if (zmalloc_thread_safe) { \
        update_zmalloc_stat_sub(_n); \
    } else { \
        used_memory -= _n; \
    } \
} while(0)

static size_t used_memory = 0;
static int zmalloc_thread_safe = 0;
pthread_mutex_t used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;

void *zmalloc(size_t size)
{
    void *ptr = malloc(size + PREFIX_SIZE);
    
    if (!ptr)
        return NULL;
    
    *((size_t*) ptr) = size;
    update_zmalloc_stat_alloc(size+PREFIX_SIZE);
    return (char*) ptr + PREFIX_SIZE;
}

void *zcalloc(size_t size)
{
    void *ptr = calloc(1, size + PREFIX_SIZE);
    
    if (!ptr)
        return NULL;
    
    *((size_t*) ptr) = size;
    update_zmalloc_stat_alloc(size+PREFIX_SIZE);
    return (char*) ptr + PREFIX_SIZE;
}

void *zrealloc(void *ptr, size_t size)
{
#ifndef HAVE_MALLOC_SIZE
    void *realptr;
#endif
    size_t oldsize;
    void *newptr;
    
    if (ptr == NULL)
        return zmalloc(size);
    
    realptr = (char*) ptr - PREFIX_SIZE;
    oldsize = *((size_t*) realptr);
    newptr = realloc(realptr, size + PREFIX_SIZE);
    if (!newptr)
        return NULL;
    
    *((size_t*) newptr) = size;
    update_zmalloc_stat_free(oldsize);
    update_zmalloc_stat_alloc(size);
    return (char*) newptr + PREFIX_SIZE;
}

/* Provide zmalloc_size() for systems where this function is not provided by
 * malloc itself, given that in that case we store a header with this
 * information as the first bytes of every allocation. */
size_t zmalloc_size(void *ptr)
{
    void *realptr = (char*) ptr - PREFIX_SIZE;
    size_t size = *((size_t*) realptr);
    /* Assume at least that all the allocations are padded at sizeof(long) by
     * the underlying allocator. */
    if (size & (sizeof(long) - 1))
        size += sizeof(long) - (size & (sizeof(long) - 1));
    return size + PREFIX_SIZE;
}

void zfree(void *ptr)
{
    void *realptr;
    size_t oldsize;
    
    if (ptr == NULL)
        return;
    
    realptr = (char*) ptr - PREFIX_SIZE;
    oldsize = *((size_t*) realptr);
    update_zmalloc_stat_free(oldsize+PREFIX_SIZE);
    free(realptr);
}

char *zstrdup(const char *s)
{
    size_t l = strlen(s) + 1;
    char *p = zmalloc(l);
    
    memcpy(p, s, l);
    return p;
}

size_t zmalloc_used_memory(void)
{
    size_t um;
    
    if (zmalloc_thread_safe) {
#if defined(HAVE_ATOMIC)
        um = update_zmalloc_stat_add(0);
#else
        pthread_mutex_lock(&used_memory_mutex);
        um = used_memory;
        pthread_mutex_unlock(&used_memory_mutex);
#endif
    }
    else {
        um = used_memory;
    }
    
    return um;
}

void zmalloc_enable_thread_safeness(void)
{
    zmalloc_thread_safe = 1;
}

/* Get the RSS information in an OS-specific way.
 *
 * WARNING: the function zmalloc_get_rss() is not designed to be fast
 * and may not be called in the busy loops where Redis tries to release
 * memory expiring or swapping out objects.
 *
 * For this kind of "fast RSS reporting" usages use instead the
 * function RedisEstimateRSS() that is a much faster (and less precise)
 * version of the function. */

#if defined(HAVE_PROC_STAT)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

size_t zmalloc_get_rss(void)
{
    int page = sysconf(_SC_PAGESIZE);
    size_t rss;
    char buf[4096];
    char filename[256];
    int fd, count;
    char *p, *x;
    
    snprintf(filename, 256, "/proc/%d/stat", getpid());
    if ((fd = open(filename, O_RDONLY)) == -1)
        return 0;
    if (read(fd, buf, 4096) <= 0) {
        close(fd);
        return 0;
    }
    close(fd);
    
    p = buf;
    count = 23; /* RSS is the 24th field in /proc/<pid>/stat */
    while (p && count--) {
        p = strchr(p, ' ');
        if (p)
            p++;
    }
    if (!p)
        return 0;
    x = strchr(p, ' ');
    if (!x)
        return 0;
    *x = '\0';
    
    rss = strtoll(p, NULL, 10);
    rss *= page;
    return rss;
}
#else
size_t zmalloc_get_rss(void)
{   
    /* If we can't get the RSS in an OS-specific way for this system just
     * return the memory usage we estimated in zmalloc()..
     *
     * Fragmentation will appear to be always 1 (no fragmentation)
     * of course... */
    return zmalloc_used_memory();
}
#endif

/* Fragmentation = RSS / allocated-bytes */
float zmalloc_get_fragmentation_ratio(size_t rss)
{
    return (float) rss / zmalloc_used_memory();
}

/* Get the sum of the specified field (converted form kb to bytes) in
 * /proc/self/smaps. The field must be specified with trailing ":" as it
 * apperas in the smaps output.
 *
 * Example: zmalloc_get_smap_bytes_by_field("Rss:");
 */
#if defined(HAVE_PROC_SMAPS)
size_t zmalloc_get_smap_bytes_by_field(char *field) {
    char line[1024];
    size_t bytes = 0;
    FILE *fp = fopen("/proc/self/smaps","r");
    int flen = strlen(field);

    if (!fp) return 0;
    while(fgets(line,sizeof(line),fp) != NULL) {
        if (strncmp(line,field,flen) == 0) {
            char *p = strchr(line,'k');
            if (p) {
                *p = '\0';
                bytes += strtol(line+flen,NULL,10) * 1024;
            }
        }
    }
    fclose(fp);
    return bytes;
}
#else
size_t zmalloc_get_smap_bytes_by_field(char *field)
{
    ((void) field);
    return 0;
}
#endif

size_t zmalloc_get_private_dirty(void)
{
    return zmalloc_get_smap_bytes_by_field("Private_Dirty:");
}
