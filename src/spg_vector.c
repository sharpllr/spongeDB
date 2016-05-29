/*
 * spg_vector.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#include "sponge.h"
#include "ds/spg_vector.h"

typedef struct vec_st {
    size_t   unitSz;
    uint32_t vecLen;
    uint32_t used;
    _vec_compare cmp;
    rwlock_t rwlock;
    struct vector vecIntf;
    uint32_t flag_order:2;
    uint32_t flag_rest:30;
    void    *data;
}vec_s;

#define DFT_VECTOR_LEN          16
#define VEC_LEN_ALIGN_MASK       8
#define VEC_SPACE_EXPAND       256

uint32_t ascend_find_pos(vec_s *vec, void *val)
{
    uint32_t idx = 0;
    char    *buf = vec->data;

    for (; idx < vec->used; idx++) {
        if (vec->cmp(val, (void*)(buf+(idx * vec->unitSz))) < 0) {
            return idx;
        }
    }

    return vec->used;
}

uint32_t descend_find_pos(vec_s *vec, void *val)
{
    uint32_t idx = 0;
    char    *buf = vec->data;

    for (; idx < vec->used; idx++) {
        if (vec->cmp(val, (void*)(buf+(idx * vec->unitSz))) > 0) {
            return idx;
        }
    }

    return vec->used;
}

uint32_t find_pos(vec_s *vec, void *val)
{
    if (vec->used == 0) {
        return 0;
    }

    //ascending order
    if (VEC_ASCEND_ORDER == vec->flag_order) {
        return ascend_find_pos(vec, val);
    }
    else if (VEC_DSCEND_ORDER == vec->flag_order) {
        return descend_find_pos(vec, val);
    }

    //Return tail of vector when vector isn't a sorted vector.
    return vec->used;
}

int32_t vec_resize(vec_s *vec)
{
    uint32_t newLen = vec->vecLen;
    newLen += ((vec->vecLen << 1) >= VEC_SPACE_EXPAND)?VEC_SPACE_EXPAND:
        (vec->vecLen << 1);

    vec->data = zrealloc(vec->data, vec->unitSz * newLen);
    if (vec->data == NULL)
        return VEC_ERROR;

    return SPG_OK;
}

static inline void vec_expand(struct vec_st *vec, uint32_t offset, uint32_t mvCnt)
{
    char *buf = vec->data;
    buf      += (vec->unitSz * offset);
    memmove(buf + (mvCnt * vec->unitSz), buf, (vec->used - offset) * vec->unitSz);
}

static inline void vec_reduce(struct vec_st *vec, uint32_t offset, uint32_t mvCnt)
{
    char *buf = vec->data;
    buf      += (vec->unitSz * offset);
    memmove(buf, buf + (mvCnt * vec->unitSz), (vec->used - offset - mvCnt) * vec->unitSz);
}

static inline void vec_set_val(struct vec_st *vec, uint32_t offset, void *val, uint32_t cnt)
{
    char *buf = vec->data;
    buf += vec->unitSz * offset;
    memcpy(buf, val, vec->unitSz * cnt);
}

int32_t  do_vec_add(struct vector *vec, void *val)
{
    int32_t  ret;
    uint32_t offset;
    vec_s *vecSt = container_of(vec, struct vec_st, vecIntf);

    //Expand space.
    if (vecSt->used == vecSt->vecLen) {
        ret = vec_resize(vecSt);
        if (ret) {
            return ret;
        }
    }

    offset = find_pos(vecSt, val);
    vec_expand(vecSt, offset, 1);
    vec_set_val(vecSt, offset, val, 1);

    vecSt->used++;

    return SPG_OK;
}

uint32_t dichotomy_find(struct vector *vec, void *val)
{
    uint32_t idx = 0;
    vec_s *vecSt = container_of(vec, struct vec_st, vecIntf);
    char  *buf   = vecSt->data;

    for (; idx < vecSt->used; idx++) {
        if (0 == vecSt->cmp(val, (void*)(buf + (idx * vecSt->unitSz)))) {
            return idx;
        }
    }

    return VEC_INFINITE_VAL_32;
}

uint32_t do_vec_find(struct vector *vec, void *val)
{
    uint32_t idx = 0;
    vec_s *vecSt = container_of(vec, struct vec_st, vecIntf);
    char  *buf   = vecSt->data;

    if (VEC_NONE_ORDER == vecSt->flag_order) {
        for (; idx < vecSt->used; idx++) {
            if (0 == vecSt->cmp(val, (void*)(buf + (idx * vecSt->unitSz)))) {
                return idx;
            }
        }
        return VEC_INFINITE_VAL_32;
    }

    return dichotomy_find(vec, val);
}

void* do_vec_get (struct vector *vec, uint32_t offset)
{
    vec_s *vecSt = container_of(vec, struct vec_st, vecIntf);
    char  *buf   = vecSt->data;

    dbg_assert(offset < vecSt->used);

    return (void*)(buf + (vecSt->unitSz * offset));
}

void* do_vec_find_get(struct vector *vec, void *val)
{
    uint32_t offset;

    offset = do_vec_find(vec, val);
    if (VEC_INFINITE_VAL_32 == offset) {
        return NULL;
    }

    return do_vec_get(vec, offset);
}

uint32_t do_vec_get_len (struct vector *vec)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    return vecSt->used;
}

int32_t  do_vec_del (struct vector *vec, uint32_t offset)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);

    if (0 == vecSt->used) {
        return SPG_OK;
    }

    dbg_assert(offset < vecSt->used);

    vec_reduce(vecSt, offset, 1);

    vecSt->used--;
    return SPG_OK;
}

void     do_read_lock(struct vector *vec)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    wrlock_getrd(&vecSt->rwlock);
}

int32_t  do_read_trylock(struct vector *vec)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    return wrlock_trygetrd(&vecSt->rwlock);
}

void     do_write_lock(struct vector *vec)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    wrlock_getwr(&vecSt->rwlock);
}

int32_t  do_write_trylock(struct vector *vec)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    return wrlock_trygetwr(&vecSt->rwlock);
}

void     do_wr_unlock(struct vector *vec)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    wrlock_unlock(&vecSt->rwlock);
}

struct vector_ops g_vec_ops = {
    ._add            = do_vec_add,
    ._find           = do_vec_find,
    ._get            = do_vec_get,
    ._del            = do_vec_del,
    ._find_get       = do_vec_find_get,
    ._get_len        = do_vec_get_len,
    ._read_lock      = do_read_lock,
    ._read_trylock   = do_read_trylock,
    ._write_lock     = do_write_lock,
    ._write_trylock  = do_write_trylock,
    ._wr_unlock      = do_wr_unlock,
};

struct vector* get_vector(size_t unitSz, _vec_compare cmp, uint32_t vecLen, uint32_t flag)
{
    struct vec_st *vec;

    if (NULL == cmp || 0 >= unitSz)
        return NULL;

    vec = zcalloc(sizeof(*vec));
    if (NULL == vec) {
        return NULL;
    }

    vec->cmp    = cmp;
    vec->unitSz = unitSz;
    vec->vecLen = (vecLen == 0)?DFT_VECTOR_LEN:ALIGN_OF(vecLen, VEC_LEN_ALIGN_MASK);
    vec->vecIntf.ops = &g_vec_ops;
    vec->flag_order  = flag;
    wrlock_init(&vec->rwlock);


    vec->data = zmalloc(vec->vecLen * vec->unitSz);
    if (NULL == vec->data) {
        zfree(vec);
        return NULL;
    }

    return &vec->vecIntf;
}

void free_vector(struct vector *vec)
{
    if (NULL == vec)
        return;

    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);

    wrlock_destroy(&vecSt->rwlock);
    zfree(vecSt->data);
    zfree(vecSt);
}

int32_t export_vector_pack(struct vector *vec, void **pack, void **data, size_t *sz)
{
    struct vec_st *vecSt = container_of(vec, struct vec_st, vecIntf);
    vec_s *newVec = zcalloc(sizeof(*newVec));

    if (NULL == newVec) {
        *pack = NULL;
        *data = NULL;
        return VEC_ERROR;
    }

    memcpy(newVec, vecSt, sizeof(*vecSt));
    newVec->vecLen  = ALIGN_OF((vecSt->used), 8);
    *data           = vecSt->data;
    *sz             = vecSt->used * vecSt->unitSz;
    *pack           = (void*)newVec;

    return SPG_OK;
}

void free_vector_pack(void *pack, void *data)
{
    vec_s *vec = (vec_s*)pack;
    //The data is bulk of memory shared with source vector.Don't free.
    (void)data;
    zfree(vec);
}

struct vector* import_vector_pack(void *pack, _vec_compare cmp, void *data, size_t sz)
{
    vec_s *imp = (vec_s*)pack;
    vec_s *vec = zmalloc(sizeof(*vec));
    if (NULL == vec) {
        return NULL;
    }

    memcpy(vec, imp, sizeof(*vec));
    wrlock_init(&vec->rwlock);
    vec->cmp  = cmp;
    vec->data = zmalloc(sz);
    if (NULL == vec->data) {
        zfree(vec);
        return NULL;
    }

    memcpy(vec->data, data, sz);

    return &vec->vecIntf;
}
