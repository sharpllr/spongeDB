// mdy000101001    20151221    Lirui.Liu Begin
/*
 * spg_util.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#ifndef SRC_SPG_UTIL_H_
#define SRC_SPG_UTIL_H_

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <endian.h>
#include <string.h>
#include "spg_predef.h"
#include "spg_type.h"
#include "ds/list.h"

#ifdef _DBG_VER_
#include <assert.h>
#define dbg_assert(expr)\
do{\
    assert((expr));\
}while(0)
#else
#define dbg_assert(expr)\
do{\
    if (!(expr)) {\
        printf("---error:[%s][%s][%d]---\n%s\n---error:[%s][%s][%d]---\n", \
            __FILE__, __func__, __LINE__, #expr, __FILE__, __func__, __LINE__);\
    }\
}while(0)
#endif


#define ALIGN_OF(x, mask) (((x) + (mask)) & ~(mask))


#define INFINITE_UINT32             ((uint32_t)(-1))
#define INFINITE_UINT64             ((uint64_t)(-1))

//Time and Timer handle
#include <sys/time.h>
int64_t get_ustime(void);
inline int64_t get_mstime(void)__attribute__((always_inline));
inline int32_t get_stime(void)__attribute__((always_inline));

#ifdef _xxx_alloc_
#elif defined(_xxx1_alloc_)
#else
#include "mem/zmalloc.h"
#endif
/*
 * sponge malloc redefine
 * */
static inline void* smalloc(size_t size)
{
    return zmalloc(size);
}

/*
 * sponge calloc redefine
 * */
static inline void* scalloc(size_t size)
{
    return zcalloc(size);
}

/*
 * sponge free redefine
 * */
static inline void sfree(void *ptr)
{
    zfree(ptr);
}

static inline char* sstrdup(char *str)
{
    return zstrdup(str);
}

#endif /* SRC_SPG_UTIL_H_ */
// mdy000101001    20151221    Lirui.Liu End
