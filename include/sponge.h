/*
 * sponge.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#ifndef SRC_SPONGE_H_
#define SRC_SPONGE_H_

#include "util/fmt_print.h"
#include "util/spg_util.h"

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define __PLATFORM_LITTLE_ENDIAN
#else
#define __PLATFORM_BIG_ENDIAN
#endif

/* BOOL define*/
#define SPG_FALSE           0
#define SPG_TRUE            1

/*Error code*/
#define SPG_OK              0
#define SPG_ERR             -1
#define SPG_OOM             1

#endif /* SRC_SPONGE_H_ */
