/*
 * spg_skiplist.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include <stdlib.h>
#include <string.h>
#include "sponge.h"
#include "util/spg_util.h"
#include "util/fmt_print.h"
#include "ds/spg_skiplist.h"

struct spg_sklnode;
#pragma pack(4)
struct skl_level {
    struct spg_sklnode *next;
};

typedef struct spg_sklnode {
    void *key;
    int32_t lvlNum;
    struct spg_sklnode *backward;
    struct skl_level lvl[0];
}sklnode_s;

#ifdef SKIPLIST_STAT
struct skl_stat {
    int32_t lvlCnt[SKIPLIST_LEVEL_MAX];
    int64_t lock_time_sum;
    int64_t ltime_tmp;
};
#endif

/*
 * @cmpFunc : Using to decide the place the new node insert
 * @level   : Max level of skiplist
 * */
struct spg_skl {
    spinlock_t lock;
    _compare *cmpFunc;
    int32_t level;
    uint32_t length;
    sklnode_s *sklhead;
    sklnode_s *skltail;
#ifdef SKIPLIST_STAT
    struct skl_stat stat;
#endif
};
#pragma pack()

#ifdef SKIPLIST_STAT
static inline sklnode_s* get_sklnode(skiplist_s *skl, int32_t level)
{
    sklnode_s *node = scalloc(sizeof(sklnode_s) + level * sizeof(struct skl_level));
    skl->stat.lvlCnt[level-1]++;
    return node;
}
static inline void put_sklnode(skiplist_s *skl, sklnode_s *node)
{
    skl->stat.lvlCnt[node->lvlNum-1]--;
    sfree(node);
}
#else
static inline sklnode_s* get_sklnode(skiplist_s *skl, int32_t level)
{
    (void)skl;
    return scalloc(sizeof(sklnode_s) + level * sizeof(struct skl_level));
}

static inline void put_sklnode(skiplist_s *skl, sklnode_s *node)
{
    (void)skl;
    sfree(node);
}
#endif


void trav_reverse(skiplist_s *skl, _do_for_key *func)
{
    sklnode_s *node = skl->skltail;
    while (NULL != node->backward) {
        func(node->key);
        node = node->backward;
    }
}

void trav_straight(skiplist_s *skl, _do_for_key *func)
{
    sklnode_s *node = skl->sklhead->lvl[0].next;

    while (NULL != node) {
        func(node->key);
        node = node->lvl[0].next;
    }
}

void skl_delete_node(skiplist_s *skl, sklnode_s *node)
{
    int32_t updLvl, idx, nodeLvl = node->lvlNum, lvlCur = 0;
    sklnode_s *back = node->backward;

    while (NULL != back) {
        updLvl = back->lvlNum-1;

        for (idx = lvlCur; idx < nodeLvl&&idx <= updLvl; idx++) {
            back->lvl[idx].next = node->lvl[idx].next;
            lvlCur++;
        }

        if (lvlCur > nodeLvl-1) {
            break;
        }
        back = back->backward;
    }
    if (NULL != node->lvl[0].next) {    //tail node have no next node
        node->lvl[0].next->backward = node->backward;
    }
    else {   //update skiplist tail
        skl->skltail = node->backward;
    }

    //Update skiplist head.
    skl->length--;
    while(1) {
        dbg_assert(skl->level > 0);
        if (skl->sklhead->lvl[skl->level-1].next != NULL) {
            break;
        }
        if (0 == --skl->level) {
            break;
        }
    }

    put_sklnode(skl, node);
}

sklnode_s* skl_get_big_equal(skiplist_s *skl, void *key, sklnode_s **prev)
{
    int32_t    lvl;
    sklnode_s *node;
    sklnode_s *head = skl->sklhead;
    sklnode_s *base = head;

    lvl = (skl->level == 0) ? 0 : skl->level - 1;

    while (1) {
        node = base->lvl[lvl].next;
        if (node != NULL && skl->cmpFunc(key, node->key) > 0) {
            base = node;
        }
        else {
            if (prev) {
                prev[lvl] = base;
            }
            if (0 == lvl) {
                return node;
            }
            lvl--;
        }
    }
    return NULL;
}

int32_t skl_gen_level(void)
{
    int32_t lvl = 1;
    int32_t rdm;
    rdm = rand() & LEVEL_RANDOM_MASK;

    while (rdm < LEVEL_HIGH_LIMIT) {
//    if (rdm < 2*LEVEL_HIGH_LIMIT) {
        lvl++;
        rdm = rand() & LEVEL_RANDOM_MASK;
    }

    return (lvl < SKIPLIST_LEVEL_MAX) ? lvl : SKIPLIST_LEVEL_MAX;
#if 0
    printf("level:");
    scanf("%d", &lvl);
    return lvl;
#endif
}

void skl_foreach_get(skiplist_s *skl, TRAV_DIR_E dir, _do_for_key *func)
{
    (void)skl;
    (void)dir;
    (void)func;
}

///////////////////////////////////////EXTERN////////////////////////////////
void skl_lock(skiplist_s *skl)
{
    spin_lock(&skl->lock);
#ifdef SKIPLIST_STAT
    skl->stat.ltime_tmp = get_mstime();
#endif
}

void skl_unlock(skiplist_s *skl)
{
    spin_unlock(&skl->lock);
#ifdef SKIPLIST_STAT
    skl->stat.lock_time_sum += (get_mstime() - skl->stat.ltime_tmp);
    skl->stat.lock_time_sum = (skl->stat.lock_time_sum > 0) ?
        skl->stat.lock_time_sum : 0;
#endif
}

int32_t skl_trylock(skiplist_s *skl)
{
#ifdef SKIPLIST_STAT
    if (spin_trylock(&skl->lock)) {
        skl->stat.ltime_tmp = get_mstime();
        return SPG_OK;
    }
    return SPG_ERR;
#else
    return spin_trylock(&skl->lock);
#endif
}

skiplist_s* skl_create(_compare *cmp)
{
    srand(time(0));
    if (NULL == cmp) {
        logErr("Compare method must be specified.");
        return NULL;
    }

    sklnode_s  *head = NULL;
    skiplist_s *skl  = smalloc(sizeof(skiplist_s));
    if (NULL == skl) {
        return NULL;
    }

    skl->length  = 0;
    skl->cmpFunc = cmp;
    skl->level   = 0;
    spin_init(&skl->lock, 0);
    skl->sklhead = get_sklnode(skl, SKIPLIST_LEVEL_MAX);
    if (NULL == skl->sklhead) {
        sfree(skl);
        return NULL;
    }
    skl->skltail = skl->sklhead;

    head = skl->sklhead;
    head->key      = NULL;
    head->lvlNum   = SKIPLIST_LEVEL_MAX;

    return skl;
}

/*
 * Destroy when skiplist is empty
 * */
void skl_destroy(skiplist_s *skl)
{
    dbg_assert(0 == skl->length);
    if (skl->length != 0) {
        logErr("Skiplist not empty,(%u) node exist.", skl->length);
        return;
    }

    sfree(skl->sklhead->lvl);
    sfree(skl->sklhead);
    sfree(skl);
}

//Insert key into skiplist
int32_t skl_insert(skiplist_s *skl, void *key)
{
    int32_t height = 0;
    sklnode_s *newNode;
    sklnode_s* prev[SKIPLIST_LEVEL_MAX] = {NULL};

    int32_t idx;
    height = skl_gen_level();
    newNode = get_sklnode(skl, height);
    if (NULL == newNode) {
        return SPG_ERR;
    }
    newNode->key = key;
    newNode->lvlNum = height;

    if (height > skl->level) {
        for (idx = skl->level; idx < height; idx++) {
            prev[idx] = skl->sklhead;
        }

        skl->level = height;
    }

    for (idx = 0; idx < height; idx++) {
        newNode->lvl[idx].next = prev[idx]->lvl[idx].next;
        memory_barrier();
        prev[idx]->lvl[idx].next = newNode;
    }

    if (NULL != newNode->lvl[0].next) {
        newNode->backward = newNode->lvl[0].next->backward;
        newNode->lvl[0].next->backward = newNode;
    }else {
        newNode->backward = skl->skltail;
        skl->skltail = newNode;
    }

    skl->length++;
    return SPG_OK;
}

//Find and delete node
void* skl_find_get(skiplist_s *skl, void *key)
{
    void *ret = NULL;
    sklnode_s *node = NULL;

    node = skl_get_big_equal(skl, key, NULL);
    if (NULL == node || node == skl->sklhead) {
        return NULL;
    }

    if (skl->cmpFunc(key, node->key) == 0) {
        ret = node->key;
        skl_delete_node(skl, node);
        return ret;
    }

    return NULL;
}

//Is key exist in skiplist
int32_t skl_contain(skiplist_s *skl, void *key)
{
    sklnode_s *node;

    node = skl_get_big_equal(skl, key, NULL);
    if (NULL == node || node == skl->sklhead) {
        return SPG_FALSE;
    }

    if (skl->cmpFunc(key, node->key) == 0)
        return SPG_TRUE;

    return SPG_FALSE;
}

//Execute func for every node
void skl_foreach(skiplist_s *skl, TRAV_DIR_E dir, _do_for_key *func)
{
    if (NULL == func || NULL == skl) {
        logErr("Parameter illegal.");
        return;
    }

    if (dir == TRAV_FORWARD)
        trav_straight(skl, func);
    else
        trav_reverse(skl, func);
}

//Delete all nodes
void skl_flush_all(skiplist_s *skl)
{
    dbg_assert(NULL != skl);
    sklnode_s *node = skl->skltail, *back = NULL;

    if (skl->length == 0) {
        return;
    }

    while (NULL != node->backward) {
        back = node->backward;
        put_sklnode(skl, node);
        node = back;
    }
    skl->length  = 0;
    skl->level   = 0;
    skl->skltail = skl->sklhead;
    memset(skl->sklhead->lvl, 0, sizeof(struct skl_level)*skl->sklhead->lvlNum);
}

#ifdef SKIPLIST_STAT
void show_skl_stat(skiplist_s *skl)
{
    int32_t idx = 0;
    logTrace("Skiplist statistics:");
    logTrace("\tskiplist have [%d] nodes.", skl->length);
    for (; idx < SKIPLIST_LEVEL_MAX; idx++) {
        logTrace("\t[%d]level have [%d] node.", idx, skl->stat.lvlCnt[idx]);
    }
    logTrace("Locked time total:[%lld].", (long long)skl->stat.lock_time_sum);
}
#endif

#ifdef _DBG_VER_

void show_sklnode(sklnode_s *node)
{
    int32_t lvl = node->lvlNum-1;
    logTrace("");
    logTrace("node info:");
    logTrace("node:\t %p", node);
    logTrace("\t level max:\t %d", node->lvlNum);
    logTrace("\t backward :\t %p", node->backward);

    logTrace("\t node next:");
    for (; lvl>=0; lvl--) {
        logTrace("\t\t node[%d]:[%p]", lvl, node->lvl[lvl].next);
    }
}

void trav_show_skl(skiplist_s *skl)
{
    sklnode_s *tmp = skl->sklhead->lvl[0].next;

    while (NULL != tmp) {
        show_sklnode(tmp);
        tmp = tmp->lvl[0].next;
    }
}

#endif
