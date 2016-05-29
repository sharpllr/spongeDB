/*
 * vector_ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include "sponge.h"
#include "ds/spg_vector.h"

int32_t int_cmp(void *lft, void *rgh)
{
    return *((int32_t*)lft)-*((int32_t*)rgh);
}

void do_vector_ut(void)
{
    int32_t *val;
    uint32_t idx;
    uint32_t len;
    int32_t array[] = {29,12,32,-14,213,4,12,43,56,43,13,4654,32,23,543,213,564,3,23,546,65,734};
    struct vector *vec;
    vec = get_vector(sizeof(int32_t), int_cmp, 9, VEC_NONE_ORDER);

    vec->ops->_write_lock(vec);
    printf("Input length = %d.\n", sizeof(array)/sizeof(array[0]));
    for (idx = 0; idx < sizeof(array)/sizeof(array[0]); idx++) {
        vec->ops->_add(vec, &array[idx]);
    }
    vec->ops->_wr_unlock(vec);

    vec->ops->_read_lock(vec);
    len = vec->ops->_get_len(vec);
    printf("Vector length = %u\n", len);
    for (idx = 0; idx < len; idx++) {
        val = (int32_t*)vec->ops->_get(vec, idx);
        printf("\tGet No.%d=[%d].\n", idx, *val);
    }
    vec->ops->_wr_unlock(vec);

    dbg_assert(0 == vec->ops->_read_trylock(vec));
    val = (int32_t*)vec->ops->_find_get(vec, &array[9]);
    if (NULL == val) {
        printf("Can't find valued.\n");
    }
    else {
        printf("Find:%d.\n", *val);
    }

    idx = vec->ops->_find(vec, &array[9]);
    vec->ops->_wr_unlock(vec);

    dbg_assert(0 == vec->ops->_write_trylock(vec));
    vec->ops->_del(vec, idx);
    vec->ops->_wr_unlock(vec);

    printf("Vector length = %u.\n", vec->ops->_get_len(vec));
}

