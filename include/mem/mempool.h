/*
 * Author : liulirui 
 * E-mail : sharpllr@163.com 
 *
 *  mempool prevent allocate and free nodes so frequently that make system's
 *  memory usage unstable.
 *  mempool records three water-levels to control the free node of this pool,
 *  the water-level of mempool must associate with system's water-level.
 *  get nodes from mempool:
 *      @get_mp_nodes
 *
 *  put nodes to mempool:
 *      @put_mp_nodes
 */
#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#include "util/spg_type.h"

//System memory water-level
typedef enum {
    SM_LVL_LOW = 0,
    SM_LVL_MID,
    SM_LVL_HIGH,
    SM_LVL_BUTT
}E_SYS_MEM_WATER_LEVEL;

typedef enum {
    E_WL_LOW = SM_LVL_LOW,
    E_WL_MID,
    E_WL_HIGH,
    E_WL_BUTT
}E_WATER_LEVEL;

typedef void* (mempool_alloc_t)(void *pool_data);
typedef void (mempool_free_t)(void *element, void *pool_data);

typedef struct mempool_s {
	spinlock_t lock;
	_s32 wl[E_WL_BUTT];
	_s32 syswl;
	_s32 freeCnt;
	_s32 slotCnt;
	void *pool_data;
	void **freeNode;
	mempool_alloc_t *alloc;
	mempool_free_t *free;
} mempool_t;


extern _s32 trylock_mp(mempool_t *mp);
extern void lock_mp(mempool_t *mp);
extern void unlock_mp(mempool_t *mp);
extern mempool_t *create_mp(_s32 min_nr, _s32 mid_nr, _s32 hig_nr,
    mempool_alloc_t *alloc_fn, mempool_free_t *free_fn, void *pool_data);
extern void destroy_mp(mempool_t *pool);
extern int get_mp_nodes(mempool_t *mp,  void **nodes, _u32 nr);
extern void put_mp_nodes(mempool_t *mp, void **nodes, _u32 nr);
extern void update_syswl(mempool_t *mp, _u32 syswl);
extern _s32 update_mpwl(mempool_t *mp, _s32 hwl, _s32 mwl, _s32 lwl);

#endif /* _MEMPOOL_H_ */
