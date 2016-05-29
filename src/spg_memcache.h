/*
 * spg_memcache.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 * SLAB to optimize the performance of frequent get and put memory node.
 */

#ifndef SRC_SPG_MEMCACHE_H_
#define SRC_SPG_MEMCACHE_H_

struct mem_cache_node {
    spinlock_t list_lock;

    struct list_head slab_partial;
    struct list_head slab_full;
    struct list_head slab_empty;

};

struct mem_cache {

};

#endif /* SRC_SPG_MEMCACHE_H_ */
