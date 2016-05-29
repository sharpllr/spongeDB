/*
 * spg_skiplist.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 * 
 */

#ifndef DS_SPG_SKIPLIST_H_
#define DS_SPG_SKIPLIST_H_

#define    SKIPLIST_STAT        //Open statistic when debug

#define SKIPLIST_LEVEL_MAX      16
#define LEVEL_RANDOM_MASK       0xFFFF
#define LEVEL_HIGH_LIMIT        (LEVEL_RANDOM_MASK>>2)       //random()

typedef enum traversal_dir {
    TRAV_FORWARD = 0,
    TRAV_BACKWARD,
    TRAV_BUTT
}TRAV_DIR_E;

//return 0  when lftKey == rghKey
//return >0 when lftKey >  rghKey
//return <0 when lftKey <  rghKey
typedef int32_t (_compare)(void *lftKey, void *rghKey);
typedef void (_do_for_key)(void *key);

typedef struct spg_skl skiplist_s;

extern void skl_lock(skiplist_s *skl);
extern void skl_unlock(skiplist_s *skl);
extern int32_t skl_trylock(skiplist_s *skl);
extern skiplist_s* skl_create(_compare *cmp);
extern void skl_destroy(skiplist_s *skl);
extern int32_t skl_insert(skiplist_s *skl, void *key);
extern void* skl_find_get(skiplist_s *skl, void *key);
extern int32_t skl_contain(skiplist_s *skl, void *key);
extern void skl_foreach(skiplist_s *skl, TRAV_DIR_E dir, _do_for_key *func);
extern void skl_flush_all(skiplist_s *skl);

#ifdef SKIPLIST_STAT
extern void show_skl_stat(skiplist_s *skl);
#endif

#ifdef _DBG_VER_
extern void trav_show_skl(skiplist_s *skl);
#endif

#endif /* DS_SPG_SKIPLIST_H_ */
