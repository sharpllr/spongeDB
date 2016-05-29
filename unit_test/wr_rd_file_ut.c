/*
 * wr_rd_file_ut.c
 *
 *  Created on: 2016年2月9日
 *      Author: liulirui
 */

#include <string.h>
#include "sponge.h"
#include "wr_file.h"

#define WF_TEST_PATH "ut_write_file.txt"

struct write_file *g_write_file;
struct append_file *g_append_file;
struct read_file *g_read_file;

void write_to_file(void)
{
    int32_t ret;
    char buf[128] = {0};
    int64_t offset;
    printf("Input offset and context.[offset:context]\n");
    scanf("%ld:%s", &offset, buf);

    ret = g_write_file->ops->_open(g_write_file);
    dbg_assert(SPG_OK == ret);

    ret = g_write_file->ops->_write(g_write_file, offset, buf, strlen(buf));
    if (SPG_OK != ret) {
        logEasy("Write to file failed.");
    }
    g_write_file->ops->_flush(g_write_file);
    g_write_file->ops->_close(g_write_file);
}

void append_to_file(void)
{
    int32_t ret;
    size_t len;
    char buf[128] = {0};
    scanf("%s", buf);
    ret = g_append_file->ops->_open(g_append_file);
    dbg_assert(SPG_OK == ret);
    len = strlen(buf);
    ret = g_append_file->ops->_append(g_append_file, buf, len);
    if (SPG_OK != ret) {
        logEasy("Append error.");
        dbg_assert(0);
    }
    g_append_file->ops->_flush(g_append_file);
    g_append_file->ops->_close(g_append_file);
}

void read_from_file(void)
{
    int32_t ret, len;
    char buf[128] = {0};
    scanf("%d", &len);
    ret = g_read_file->ops->_open(g_read_file);
    dbg_assert(SPG_OK == ret);
    ret = g_read_file->ops->_read(g_read_file, buf, len);
    if (len != ret) {
        logEasy("Read abnormal.");
    }
    g_read_file->ops->_close(g_read_file);
    printf("Get file context:%s\n", buf);
}

void rdm_read_from_file(void)
{
    int32_t ret, len;
    int64_t offset;
    char buf[128] = {0};
    printf("Input bias and length:->");
    scanf("%ld:%d", &offset, &len);
    ret = g_read_file->ops->_open(g_read_file);
    dbg_assert(SPG_OK == ret);
    ret = g_read_file->ops->_bias_read(g_read_file, offset, buf, len);
    if (len != ret) {
        logEasy("Read abnormal.");
    }
    g_read_file->ops->_close(g_read_file);
    printf("Get file context:%s\n", buf);
}

void do_fileio_ut(void)
{
    struct file_attr attr;
    attr.path     = WF_TEST_PATH;
    g_write_file  = create_wfile(&attr);
    dbg_assert(NULL != g_write_file);
    g_read_file   = create_rfile(&attr);
    dbg_assert(NULL != g_read_file);
    g_append_file = create_afile(&attr);
    dbg_assert(NULL != g_append_file);

    uint32_t item;
    while(1) {
        printf("UT item:\n"\
            "\t1. Append file input.\n"\
            "\t2. Write file write.\n"\
            "\t3. Sequence read file read.\n"\
            "\t4. Random read file.\n");
        scanf("%u", &item);
        switch(item) {
            case 1:
                append_to_file();
                break;
            case 2:
                write_to_file();
                break;
            case 3:
                read_from_file();
                break;
            case 4:
                rdm_read_from_file();
                break;
            default:
                return;
        }
    }

}
