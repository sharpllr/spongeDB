/*
 * skiplist_ut.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#include <unistd.h>
#include "ut.h"

#include "ds/spg_skiplist.h"
//#define STATIC_TEST
skiplist_s *g_skl;

#if 0
int32_t test_array[] = {91, 4, 24};
int32_t test_array[] = {98, 89, 80, 50, 43, 43, 38, 21, 21, 19, 16, 14, 12};
#endif
int32_t test_array[] = {
    91, 75,  1,  4, 24, 14, 32, 45, 81, 72,
    19, 86, 30, 66, 28, 71, 51, 55, 2,  23,
    55, 37, 62, 98, 7,  31, 84, 29, 42, 44
};

void print_val(void *val)
{
    printf("%d ", *((int32_t*)val));
}

int32_t key_compare(void *lftKey, void *rghKey)
{
    if (NULL == lftKey || NULL == rghKey) {
        trav_show_skl(g_skl);
    }

    return *((int32_t*)lftKey) - *((int32_t*)rghKey);
}

void do_skl_insert(void)
{

#ifdef STATIC_TEST  //dead test
    int32_t ret;
    int32_t cnt = sizeof(test_array)/sizeof(test_array[0]) - 1;

    for (; cnt >= 0; cnt--) {
        ret = skl_insert(g_skl, &test_array[cnt]);
        if (SPG_OK != ret) {
            logErr("Skiplist insert failed.");
            return;
        }
    }

#else
    int *val, ret;
    printf("input value:\n\t");
    val = smalloc(sizeof(*val));
    dbg_assert(NULL != val);
    scanf("%d", val);
    ret = skl_insert(g_skl, val);
    if (SPG_OK != ret) {
        logErr("Insert failed.");
        return ;
    }
#endif
    logEasy("Insert success.");
}

void do_skl_traversal_forward(void)
{
    logEasy("Forward traversal:");
    skl_foreach(g_skl, TRAV_FORWARD, print_val);
    printf("\n");
}

void do_skl_traversal_backward(void)
{
    logEasy("Backward traversal:");
    skl_foreach(g_skl, TRAV_BACKWARD, print_val);
    printf("\n");
}

void do_skl_find(void)
{
#ifdef STATIC_TEST
    int32_t *ret;
    int32_t cnt = sizeof(test_array)/sizeof(test_array[0]) - 1;

    for (; cnt >= 0; cnt--) {
        ret = (int*)skl_find_get(g_skl, &test_array[cnt]);
        if (NULL == ret) {
            logEasy("Not found.");
            return;
        }
    }
#else
    int val, *ret;
    printf("input value:\n\t");
    scanf("%d", &val);
    ret = (int*)skl_find_get(g_skl, &val);
    if (NULL == ret) {
        printf("not found\n");
        return;
    }
    printf("found value\n");
    sfree(ret);
#endif
    logEasy("Find and get success.");
}

void do_times_insert(uint32_t times)
{
    int32_t *key;
    uint32_t idx = 0;

    for (; idx < times; idx++) {
        key = smalloc(sizeof(*key));
        *key = rand()%300;

        if (SPG_OK != skl_insert(g_skl, key)) {
            logEasy("Insert failed.");
        }
    }
}

void do_times_find_get(uint32_t times)
{
    int32_t key, *retKey;
    uint32_t idx = 0;
    for (; idx < times; idx++) {
        key = rand()%300;
        retKey = skl_find_get(g_skl, &key);
        if (NULL != retKey) {
            logEasy("found key=%d.", key);
            sfree(retKey);
        }
    }
}

void do_random_test(void)
{
    uint32_t times;
    uint32_t loop;
    uint32_t in;
    scanf("%u", &in);
    while(in) {
        --in;
        loop = rand()%10;
        while (loop--) {
            times = rand()%150;
            do_times_insert(times);
            do_times_find_get(3*times);
            logEasy("Do %u times test complete.", times);
        }
        show_skl_stat(g_skl);
        sleep(1);
    }
}

void do_skl_ut(void)
{
    g_skl = skl_create(key_compare);
    printf("Unit test item:\n\t1.insert item,input int value.\n"\
        "\t2.traversal skiplist forward.\n"\
        "\t3.traversal skiplist backward.\n"\
        "\t4.find and delete item.\n"\
        "\t5.flushall skiplist nodes.\n"\
        "\t6.show skiplist statistics.\n"\
        "\t7.do loop random test.\n"\
        "\t8.show all node info.\n"\
        "\t9.show help.\n");
    int item;
    while(1) {
        scanf("%d", &item);
        switch (item) {
            case 1:
                do_skl_insert();
                break;
            case 2:
                do_skl_traversal_forward();
                break;
            case 3:
                do_skl_traversal_backward();
                break;
            case 4:
                do_skl_find();
                break;
            case 5:
                skl_flush_all(g_skl);
                break;
            case 6:
                show_skl_stat(g_skl);
                break;
            case 7:
                do_random_test();
                break;
            case 8:
                trav_show_skl(g_skl);
                break;
            case 9:
                printf("Unit test item:\n\t1.insert item,input int value.\n"\
                    "\t2.traversal skiplist forward.\n"\
                    "\t3.traversal skiplist backward.\n"\
                    "\t4.find and delete item.\n"\
                    "\t5.flushall skiplist nodes.\n"\
                    "\t6.show skiplist statistics.\n"\
                    "\t7.do loop random test.\n"\
                    "\t8.show all node info.\n"\
                    "\t9.show help.\n");
                break;
            default:
                break;
        }
    }
}
