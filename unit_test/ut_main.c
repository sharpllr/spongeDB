/*
 * main.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */
#include "ut.h"

int main(int argc, char** argv)
{
//    do_mem_ut();    //memory operations test
//    show_sds();     //sds operation test
//    do_sha1();      //Do SHA hash
#ifdef _UT_CLI_
    //launch_client(argc, argv);
#else
    //launch_srv();
#endif

//    ut_mempool();
//----atomic---test----
//    atomic_ut();
//    atomic_pef_ut();
//----skiplist-test----
//    do_skl_ut();
//    do_crc32_ut();

//    do_fileio_ut();

//    init_gossip_svc();
    do_vector_ut();

    return 0;
}

