/*
 * spg_ref.h
 *
 *  Created on: 2016年2月6日
 *      Author: liulirui
 */

#ifndef INCLUDE_UTIL_SPG_REF_H_
#define INCLUDE_UTIL_SPG_REF_H_

#include "spg_util.h"

typedef atomic_t ref_t;

static inline void ref_init(ref_t *ref)
{
    atomic_set(ref, 1);
}

static inline void ref_get(ref_t *ref)
{
    (void)atomic_inc_return(ref);
}

/*
 * ref_put - decrement refcount for object.
 * Return 1 if the object was removed, otherwise return 0.
 * */
static inline int ref_put(ref_t *ref, void (*release)(ref_t *ref))
{
    dbg_assert(NULL != release);
    if (1 == atomic_dec_return(ref)) {
        release(ref);
        return 1;
    }
    return 0;
}


#endif /* INCLUDE_UTIL_SPG_REF_H_ */
