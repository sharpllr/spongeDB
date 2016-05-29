/*
 * mem_ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include "ut.h"
#include "mem/zmalloc.h"

void do_mem_ut(void)
{
    unsigned long usageLen;
    char *buf = NULL;
    float rate;

    usageLen = zmalloc_get_rss();
    printf("Before alloc,usage length = %lu.\n", usageLen);

    buf = zmalloc(1024*100);
    memset(buf, 0, 1024);

    usageLen = zmalloc_get_rss();
    printf("After alloc,usage length = %lu.\n", usageLen);

    rate = zmalloc_get_fragmentation_ratio(usageLen);
    printf("Ration = %f\n", rate);

    zfree(buf);
}
