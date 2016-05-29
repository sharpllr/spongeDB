/*
 * spg_type.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#ifndef SRC_SPG_TYPE_H_
#define SRC_SPG_TYPE_H_

typedef unsigned char uchar_t;
typedef int _s32;
typedef unsigned int _u32;
typedef unsigned long long _u64;
typedef long long _s64;
typedef int bool_t;

///Steal from leveldb ^_-
typedef struct data_pack {
    char *data;
    _u32  len;
}slice_s;

typedef uint64_t hash_t;

#ifdef _POSIX_

#include <assert.h>
#include <pthread.h>
/*
 * read/write lock
 * */
typedef pthread_rwlock_t rwlock_t;

static inline int32_t wrlock_init(rwlock_t *lock)
{
    return pthread_rwlock_init(lock, NULL);
}

static inline void wrlock_destroy(rwlock_t *lock)
{
    (void)pthread_rwlock_destroy(lock);
}

static inline void wrlock_getwr(rwlock_t *lock)
{
    (void)pthread_rwlock_wrlock(lock);
}

static inline void wrlock_getrd(rwlock_t *lock)
{
    (void)pthread_rwlock_rdlock(lock);
}

static inline int32_t wrlock_trygetwr(rwlock_t *lock)
{
    return pthread_rwlock_trywrlock(lock);
}

static inline int32_t wrlock_trygetrd(rwlock_t *lock)
{
    return pthread_rwlock_tryrdlock(lock);
}

static inline void wrlock_unlock(rwlock_t *lock)
{
    (void)pthread_rwlock_unlock(lock);
}

/*
 * mutex lock
 * */
typedef pthread_mutex_t mutex_t;
static inline void mutex_lock(mutex_t *mtx)
{
#ifdef _DBG_VER_
    int32_t ret = pthread_mutex_lock(mtx);
    if (ret) {
        assert(0);
    }
#else
    (void)pthread_mutex_lock(mtx);
#endif
}
static inline int32_t mutex_trylock(mutex_t *mtx)
{
    return pthread_mutex_trylock(mtx);
}
static inline void mutex_unlock(mutex_t *mtx)
{
#ifdef _DBG_VER_
    int32_t ret = pthread_mutex_unlock(mtx);
    if (ret) {
        assert(0);
    }
#else
    (void)pthread_mutex_unlock(mtx);
#endif
}

static inline int32_t mutex_init(mutex_t *mtx)
{
    pthread_mutexattr_t attr;
#ifdef _DBG_VER_
    attr = (pthread_mutexattr_t)PTHREAD_MUTEX_ERRORCHECK;
    return pthread_mutex_init(mtx, &attr);
#else
    return pthread_mutex_init(mtx, NULL);
#endif
}

static inline void mutex_destroy(mutex_t *mtx)
{
    pthread_mutex_destroy(mtx);
}

/*
 * spinlock
 * */
typedef pthread_spinlock_t  spinlock_t;
static inline _s32 spin_init(spinlock_t *lock, _s32 shared)
{
    return pthread_spin_init(lock, shared);
}

static inline _s32 spin_lock(spinlock_t *lock)
{
    //(lock) will failed because of deadlock
    return pthread_spin_lock(lock);
}

static inline _s32 spin_trylock(spinlock_t *lock)
{
    return pthread_spin_trylock(lock);
}

static inline _s32 spin_unlock(spinlock_t *lock)
{
    return pthread_spin_unlock(lock);
}

static inline _s32 spin_destroy(spinlock_t *lock)
{
    return pthread_spin_destroy(lock);
}
#else
typedef _s32                    spinlock_t;
static inline _s32 spin_init(spinlock_t *lock, _s32 shared)
{
    return -1;
}

static inline _s32 spin_lock(spinlock_t *lock)
{
    //(lock) will failed because of deadlock
    return -1;
}

static inline _s32 spin_trylock(spinlock_t *lock)
{
    return -1;
}

static inline _s32 spin_unlock(spinlock_t *lock)
{
    return -1;
}

static inline _s32 spin_destroy(spinlock_t *lock)
{
    return -1;
}
#endif

#ifdef _GCC_BUILT_ATOMIC_
typedef int32_t atomic_t;
static inline void atomic_set(atomic_t *dst, int32_t val)
{
    *dst = val;
}

static inline void atomic_add(atomic_t *dst, int32_t val)
{
    (void)__sync_add_and_fetch(dst, val);
}

static inline void atomic_sub(atomic_t *dst, int32_t val)
{
    (void)__sync_sub_and_fetch(dst, val);
}

static inline int32_t atomic_add_return(atomic_t *dst, int32_t val)
{
    return __sync_add_and_fetch(dst, val);
}

static inline int32_t atomic_sub_return(atomic_t *dst, int32_t val)
{
    return __sync_sub_and_fetch(dst, val);
}

static inline void atomic_inc(atomic_t *val)
{
    (void)__sync_add_and_fetch(val, 1);
}

static inline void atomic_dec(atomic_t *val)
{
    (void)__sync_sub_and_fetch(val, 1);
}

static inline int32_t atomic_inc_return(atomic_t *val)
{
    return __sync_add_and_fetch(val, 1);
}

static inline int32_t atomic_dec_return(atomic_t *val)
{
    return __sync_sub_and_fetch(val, 1);
}

#ifdef __ARCH_CPU_X86_FAMILY__
static inline void memory_barrier(void)
{
    __asm__ __volatile__("" : : : "memory");
}
#else
static inline void memory_barrier(void)
{
    return;
}
#endif

#elif defined(_POSIX_MUTEX_ATOMIC_)
typedef int32_t atomic_t;
void atomic_set(atomic_t *dst, int32_t val);
void atomic_add(atomic_t *dst, int32_t val);
void atomic_sub(atomic_t *dst, int32_t val);
int32_t atomic_add_return(atomic_t *dst, int32_t val);
int32_t atomic_sub_return(atomic_t *dst, int32_t val);
void atomic_inc(atomic_t *val);
void atomic_dec(atomic_t *val);
int32_t atomic_inc_return(atomic_t *val);
int32_t atomic_dec_return(atomic_t *val);
void memory_barrier(void);

#else

#endif

#endif /* SRC_SPG_TYPE_H_ */
