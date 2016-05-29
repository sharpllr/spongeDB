/*
 * ut.h
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#ifndef UNIT_TEST_UT_H_
#define UNIT_TEST_UT_H_
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "util/fmt_print.h"
#include "util/spg_util.h"
#include "util/spg_type.h"
#include "sponge.h"


void do_mem_ut(void);
void show_sds(void);
void do_sha1();
void hex_dump(char *info, unsigned char *data, uint32_t len);
void ut_mempool(void);
void do_alloc_free_test(uint32_t round);

#ifdef _UT_CLI_
void launch_client(int argc, char** argv);
#else
void launch_srv(void);
#endif

//////////////////////////ATOMIC TEST////////////////////////////
extern void atomic_ut(void);
extern void atomic_pef_ut(void);


/////////////////////////SKIPLIST TEST///////////////////////////
extern void do_skl_ut(void);

/////////////////////////WRITE FILE TEST///////////////////////////
extern void do_fileio_ut(void);

/////////////////////////VECTOR TEST///////////////////////////
extern void do_vector_ut(void);
#endif /* UNIT_TEST_UT_H_ */
