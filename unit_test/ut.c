/*
 * ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include "ut.h"

void hex_dump(char *info, unsigned char *data, uint32_t len)
{
    uint32_t i = 0;
    char tmp[128] = {0};
    if (NULL == data) {
        return;
    }
    logEasy("[%s length=%u]:", info, len);
    for (i = 0; i < len; i++) {
        sprintf(tmp+((i&0xF)*3), "%02X ", data[i]);
        if (((i+1)&0xF) == 0) {
            logEasy("%s", tmp);
            memset(tmp, 0, sizeof(tmp));
        }
    }
    if (0 != ((i+1)&0xF)) {
        logEasy("%s", tmp);
    }
}
