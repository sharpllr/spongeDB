/*
 * spg_util.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#include <stdio.h>
#include "util/spg_util.h"
int64_t get_ustime(void)
{
    struct timeval tv;
    int64_t ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

int64_t get_mstime(void)
{
    return get_ustime()/1000;
}

int32_t get_stime(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (int32_t)tv.tv_sec;
}
