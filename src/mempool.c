/*
 * mempool.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include <string.h>
#include "sponge.h"
#include "util/spg_util.h"

#include "mem/mempool.h"


static inline void *remove_element(mempool_t *mp)
{
    dbg_assert(mp->freeCnt > 0);
    return mp->freeNode[--mp->freeCnt];
}

static inline void add_element(mempool_t *pool, void *element)
{
    dbg_assert(pool->freeCnt < pool->wl[pool->syswl]);
    pool->freeNode[pool->freeCnt++] = element;
}

void resize_mp(mempool_t *mp, _s32 force)
{
    void *newSlots;
    _s32 slots = mp->wl[mp->syswl];

    if (mp->slotCnt < slots && !force)    //Do resize when freeNode count reach the slotCnt
        return;

    if (mp->freeCnt > slots) {
        dbg_assert(mp->freeCnt <= slots);
        return ;
    }

    newSlots = smalloc(slots * sizeof(void*));
    if (!newSlots)
        return;

    memcpy(newSlots, mp->freeNode, slots * sizeof(void*));
    sfree(mp->freeNode);
    mp->freeNode = newSlots;
    mp->slotCnt  = slots;
}

void destroy_mp(mempool_t *mp)
{
    while (mp->freeCnt) {
        void *element = remove_element(mp);
        mp->free(element, mp->pool_data);
    }
    sfree(mp->freeNode);
    sfree(mp);
}

mempool_t *create_mp(_s32 low_nr, _s32 mid_nr, _s32 hig_nr,
    mempool_alloc_t *alloc_fn, mempool_free_t *free_fn,
    void *pool_data)
{
    void *tmp;
    mempool_t *mp;
    mp = scalloc(sizeof(*mp));
    if (!mp)
        return NULL;

    mp->freeNode = scalloc(sizeof(void*) * low_nr);
    if (!mp->freeNode) {
        sfree(mp);
        return NULL;
    }

    spin_init(&mp->lock, 0);

    mp->slotCnt         = low_nr;
    mp->wl[E_WL_LOW]    = low_nr;
    mp->wl[E_WL_MID]    = mid_nr;
    mp->wl[E_WL_HIGH]   = hig_nr;
    mp->free            = free_fn;
    mp->alloc           = alloc_fn;
    mp->pool_data       = pool_data;
    mp->syswl           = SM_LVL_LOW;

    while (mp->freeCnt < mp->wl[mp->syswl]) {
        tmp = mp->alloc(mp->pool_data);
//        if (mp->freeNode[mp->freeCnt]) {
        if (!tmp) {
            destroy_mp(mp);
            return NULL;
        }
        add_element(mp, tmp);
    }

    return mp;

}

void clean_node_list(mempool_t *mp, void **nodes, _u32 nr)
{
    int idx;

    for (idx = 0; idx < nr; idx++) {
        if (nodes[idx] == NULL)
            return;
        put_mp_nodes(mp, &nodes[idx], 1);
        nodes[idx] = NULL;
    }
}

static inline void* get_resrv_node(mempool_t *mp)
{
    if (mp->freeCnt <= 0) {
        return NULL;
    }

    return remove_element(mp);
}

int get_mp_nodes(mempool_t *mp,  void **nodes, _u32 nr)
{
    int idx;
    void *tmp;

    for (idx = 0; idx < nr; idx++) {
        tmp = mp->alloc(mp->pool_data);
        if (NULL == tmp) {
            tmp = get_resrv_node(mp);
            if (NULL == tmp) {
                clean_node_list(mp, nodes, nr);
                return SPG_ERR;
            }
        }
        nodes[idx] = tmp;
    }
    return SPG_OK;
}

void put_mp_nodes(mempool_t *mp, void **nodes, _u32 nr)
{
    _s32 idx = 0;
    _s32 lvlCnt = mp->wl[mp->syswl];

    if (mp->freeCnt + nr > mp->slotCnt) {   //Extend node slots when necessary
        resize_mp(mp, SPG_TRUE);
    }

    if (mp->freeCnt + nr <= lvlCnt) {
        for (; idx < nr; idx++) {
            add_element(mp, nodes[idx]);
        }
        return;
    }

    //two part:some of throw into list,some of free
    _s32 freeCnt = lvlCnt - mp->freeCnt;
    for (; idx < freeCnt; idx++) {
        add_element(mp, nodes[idx]);
    }

    for (; idx < nr; idx++) {
        mp->free(nodes[idx], mp->pool_data);
    }

}

void update_syswl(mempool_t *mp, _u32 syswl)
{
    if (mp->syswl == syswl)
        return;

    mp->syswl = syswl;

    if (mp->freeCnt <= mp->wl[syswl])
        return;

    _s32 needFree = mp->freeCnt - mp->wl[syswl];
    void *node;

    while (needFree--) {
        node = remove_element(mp);
        mp->free(node, mp->pool_data);
    }

    resize_mp(mp, SPG_FALSE);
}

static inline void trim_fnode(mempool_t *mp)
{
    void *node;
    while (mp->freeCnt > mp->wl[mp->syswl]) {
        node = remove_element(mp);
        mp->free(node, mp->pool_data);
    }
}

/*
 * Update mempool's water-levels.
 * */
_s32 update_mpwl(mempool_t *mp, _s32 hwl, _s32 mwl, _s32 lwl)
{
    mp->wl[E_WL_HIGH] = hwl;
    mp->wl[E_WL_MID]  = mwl;
    mp->wl[E_WL_LOW] = lwl;
    trim_fnode(mp);
    resize_mp(mp, SPG_FALSE);
    return SPG_OK;
}

_s32 trylock_mp(mempool_t *mp)
{
    return spin_trylock(&mp->lock);
}

void lock_mp(mempool_t *mp)
{
    spin_lock(&mp->lock);
}

void unlock_mp(mempool_t *mp)
{
    spin_unlock(&mp->lock);
}
