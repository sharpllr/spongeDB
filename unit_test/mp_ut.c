/*
 * mp_ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#include "ut.h"
#include "mem/zmalloc.h"
#include "mem/mempool.h"

#define DO_ALLOC_FREE_TEST_TIMES    40

mempool_t *g_mp;

struct lalala {
    int32_t mgList[100];
};

void* alloc_unit(void *pool_data)
{
    int idx = 0;
    struct lalala *la = zmalloc(sizeof(struct lalala));
    for (; idx < 100; idx++) {
        la->mgList[idx] = idx;
    }

    return la;
}

void free_unit(void *element, void *pool_data)
{
    int idx = 0;
    struct lalala *la = element;
    for (; idx < 100; idx++) {
        if (la->mgList[idx] != idx) {
            logEasy("Oh..No..Memory Error.WTF You Coded.");
        }
    }
    zfree(element);
}

void vfy_itm(struct lalala **la, uint32_t cnt)
{
    _u32 i;
    struct lalala *ll;
    for (i = 0; i < cnt; i++) {
        int idx = 0;
        ll = la[i];
        for (; idx < 100; idx++) {
            if (ll->mgList[idx] != idx) {
                logEasy("Oh..No..Memory Error.WTF You Coded.");
            }
        }
    }
}

void mdy_mp_wl(void)
{
    int hg, md, lw;

    printf("Input high\\middle\\low water level.\n");
    scanf("%d %d %d", &hg, &md, &lw);

    update_mpwl(g_mp, hg, md, lw);
}

void do_alloc_free_test(uint32_t round)
{
    struct lalala *tmp[10];

    while(round--) {
        if (SPG_OK != get_mp_nodes(g_mp, (void**)tmp, 10)) {
            logEasy("Oh...No...Get nodes failed.");
            return;
        }
        logEasy("After get:mempool info:freeCnt = %d,highlv = %d, midlv = %d, lowlv = %d.",
            g_mp->freeCnt, g_mp->wl[E_WL_HIGH], g_mp->wl[E_WL_MID], g_mp->wl[E_WL_LOW]);
        logEasy("             slots = %d.", g_mp->slotCnt);

        mdy_mp_wl();
        logEasy("After change level:mempool info:freeCnt = %d,highlv = %d, midlv = %d, lowlv = %d.",
            g_mp->freeCnt, g_mp->wl[E_WL_HIGH], g_mp->wl[E_WL_MID], g_mp->wl[E_WL_LOW]);
        logEasy("             slots = %d.", g_mp->slotCnt);
        put_mp_nodes(g_mp, (void**)tmp, 10);
        logEasy("After put:mempool info:freeCnt = %d,highlv = %d, midlv = %d, lowlv = %d.",
            g_mp->freeCnt, g_mp->wl[E_WL_HIGH], g_mp->wl[E_WL_MID], g_mp->wl[E_WL_LOW]);
        logEasy("             slots = %d.", g_mp->slotCnt);
    }
}

void ut_mempool(void)
{
    uint32_t round = DO_ALLOC_FREE_TEST_TIMES;
    g_mp = create_mp(128, 512, 1024, alloc_unit, free_unit, NULL);
    if (NULL == g_mp) {
        logEasy("Create Mempool Failed.WTF.");
        return;
    }

    do_alloc_free_test(round);
    destroy_mp(g_mp);
}
