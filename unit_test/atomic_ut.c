/*
 * atomic_ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#include "ut.h"

void atomic_ut(void)
{
    atomic_t t1;
    int32_t ret;

    atomic_set(&t1, 100);

    atomic_add(&t1, 100);
    ret = atomic_add_return(&t1, 100);
    logEasy("get val = %d.", ret);

    atomic_sub(&t1, 100);
    ret = atomic_sub_return(&t1, 100);
    logEasy("get val = %d.", ret);

    atomic_inc(&t1);
    ret = atomic_inc_return(&t1);
    logEasy("get val = %d.", ret);

    atomic_dec(&t1);
    ret = atomic_dec_return(&t1);
    logEasy("get val = %d.", ret);

    t1 = 0;
    memory_barrier();
    ret = t1+1;
    logEasy("get val = %d.", ret);

}

void atomic_pef_ut(void)
{
    int32_t i = 0;
    int64_t us;
    atomic_t t1;
    atomic_set(&t1, 0);
    us = get_ustime();
    while (i < 1000000) {
        atomic_inc(&t1);
//        t1++;
        i++;
    }
    us = get_ustime() - us;

    logEasy("Using time:%lld us", (long long)us);
}
