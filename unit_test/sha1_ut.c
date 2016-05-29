/*
 * sha1_ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include "ut.h"
#include "sha1.h"


void do_sha1(void)
{
    SHA1_CTX ctx;
    char *data = "abc";
    char hash[20];
    uint32_t len = strlen(data);
    SHA1Init(&ctx);
    SHA1Update(&ctx, (unsigned char*)data, len);
    SHA1Final((unsigned char*)hash, &ctx);
    hex_dump("sha1 of \"abc\":", (unsigned char*)hash, sizeof(hash));
}
