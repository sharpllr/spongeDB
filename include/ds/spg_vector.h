/*
 * spg_vector.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 * Sponge vector is sorted vector.When get a vector,must provides a
 * compare method to comparing two value.
 *
 */

#ifndef INCLUDE_DS_SPG_VECTOR_H_
#define INCLUDE_DS_SPG_VECTOR_H_

#define VEC_NONE_ORDER               0          //No sort
#define VEC_ASCEND_ORDER             1          //Ascending order
#define VEC_DSCEND_ORDER             2          //Descending order

#define VEC_ERROR                   -1
#define VEC_EMPTY                    2

#define VEC_INFINITE_VAL_32         INFINITE_UINT32

struct vector;
//return (*lft)-(*rgh)
typedef int32_t (*_vec_compare)(void *lft, void *rgh);

struct vector_ops {
    int32_t  (*_add)     (struct vector *vec, void *val);
    uint32_t (*_find)    (struct vector *vec, void *val);
    void*    (*_get)     (struct vector *vec, uint32_t offset);
    int32_t  (*_del)     (struct vector *vec, uint32_t offset);
    void*    (*_find_get)(struct vector *vec, void *val);
    uint32_t (*_get_len) (struct vector *vec);
    void     (*_read_lock)(struct vector *vec);
    int32_t  (*_read_trylock)(struct vector *vec);
    void     (*_write_lock)(struct vector *vec);
    int32_t  (*_write_trylock)(struct vector *vec);
    void     (*_wr_unlock)(struct vector *vec);
};

struct vector {
    struct vector_ops *ops;
};

struct vector* get_vector(size_t unitSz, _vec_compare cmp,
    uint32_t vecLen, uint32_t flag);

void free_vector(struct vector *vec);

int32_t export_vector_pack(struct vector *vec, void **pack,
    void **data, size_t *sz);

void free_vector_pack(void *pack, void *data);

struct vector* import_vector_pack(void *pack, _vec_compare cmp,
    void *data, size_t sz);

#endif /* INCLUDE_DS_SPG_VECTOR_H_ */
