/*
 * spg_atomic.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */


#ifdef _POSIX_MUTEX_ATOMIC_
#include "util/spg_type.h"

pthread_mutex_t g_atomic_mutex = PTHREAD_MUTEX_INITIALIZER;

void atomic_set(atomic_t *dst, int32_t val)
{
    pthread_mutex_lock(&g_atomic_mutex);
    *dst = val;
    pthread_mutex_unlock(&g_atomic_mutex);
}

void atomic_add(atomic_t *dst, int32_t val)
{
    pthread_mutex_lock(&g_atomic_mutex);
    *dst += val;
    pthread_mutex_unlock(&g_atomic_mutex);
}

void atomic_sub(atomic_t *dst, int32_t val)
{
    pthread_mutex_lock(&g_atomic_mutex);
    *dst -= val;
    pthread_mutex_unlock(&g_atomic_mutex);
}

int32_t atomic_add_return(atomic_t *dst, int32_t val)
{
    int32_t ret;
    pthread_mutex_lock(&g_atomic_mutex);
    *dst += val;
    ret   = *dst;
    pthread_mutex_unlock(&g_atomic_mutex);
    return ret;
}

int32_t atomic_sub_return(atomic_t *dst, int32_t val)
{
    int32_t ret;
    pthread_mutex_lock(&g_atomic_mutex);
    *dst -= val;
    ret   = *dst;
    pthread_mutex_unlock(&g_atomic_mutex);
    return ret;
}

void atomic_inc(atomic_t *val)
{
    pthread_mutex_lock(&g_atomic_mutex);
    (*val)++;
    pthread_mutex_unlock(&g_atomic_mutex);
}

void atomic_dec(atomic_t *val)
{
    pthread_mutex_lock(&g_atomic_mutex);
    (*val)--;
    pthread_mutex_unlock(&g_atomic_mutex);
}

int32_t atomic_inc_return(atomic_t *val)
{
    int32_t ret;
    pthread_mutex_lock(&g_atomic_mutex);
    (*val)++;
    ret = *val;
    pthread_mutex_unlock(&g_atomic_mutex);
    return ret;
}

int32_t atomic_dec_return(atomic_t *val)
{
    int32_t ret;
    pthread_mutex_lock(&g_atomic_mutex);
    (*val)--;
    ret = *val;
    pthread_mutex_unlock(&g_atomic_mutex);
    return ret;
}

void memory_barrier(void)
{
    return;
}
#endif
