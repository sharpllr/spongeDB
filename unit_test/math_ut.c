/*
 * math_ut.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#include "ut.h"
#include "alg_math.h"

static uint32_t ut_do_crc32(void *data, uint32_t length)
{
    uint32_t crc = 0;
    crc = get_crc32(crc, (char*)data, length);
    return crc;
}

void do_crc32_ut(void)
{
    char data[128] = "hello world";
    uint32_t crcRet;

//    uint32_t len;
    //scanf("%s", data);
    //scanf("%u", &len);

    crcRet = ut_do_crc32(data, strlen(data));
    logEasy("Do crc32 ret = 0x%x.", crcRet);
}
