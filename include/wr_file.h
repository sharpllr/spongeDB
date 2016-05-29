/*
 * wr_file.h
 *
 *  Provide the writable and readable file and operations.
 *  Created on: 2016年2月6日
 *      Author: liulirui
 */

#ifndef INCLUDE_WR_FILE_H_
#define INCLUDE_WR_FILE_H_

//Macro configure
#ifdef _DBG_VER_
#define _CHECK_ERR_DETAIL_ 1
#endif

#if _CHECK_ERR_DETAIL_
#define WRFILE_RET_LOCK_OCCUPIED   0xf1
#endif

struct write_file;
struct append_file;
struct read_file;

//Random write file object
struct wf_ops {
    int32_t (*_write)(struct write_file *file, int64_t offset,
        void *data, size_t len);
    int32_t (*_open)(struct write_file *file);
    int32_t (*_close)(struct write_file *file);
    int32_t (*_flush)(struct write_file *file);
    void    (*_wrlock)(struct write_file *file);
    int32_t (*_try_wrlock)(struct write_file *file);
    void    (*_unlock)(struct write_file *file);
};

struct write_file {
    struct wf_ops *ops;
};

//Random read file object
struct rf_ops {
    int32_t (*_open)(struct read_file *file);
    int32_t (*_close)(struct read_file *file);
    size_t (*_bias_read)(struct read_file *file, int64_t offset,
        void *buf, size_t len);
    size_t (*_read)(struct read_file *file, void *buf, size_t len);
    void    (*_rdlock)(struct read_file *file);
    int32_t (*_try_rdlock)(struct read_file *file);
    void    (*_unlock)(struct read_file *file);
};

struct read_file {
    struct rf_ops *ops;
};

//Append file object
struct af_ops {
    int32_t (*_append)(struct append_file *file, void *data, size_t len);
    int32_t (*_open)(struct append_file *file);
    int32_t (*_close)(struct append_file *file);
    int32_t (*_flush)(struct append_file *file);
    void    (*_aplock)(struct append_file *file);
    int32_t (*_try_aplock)(struct append_file *file);
    void    (*_unlock)(struct append_file *file);
};
struct append_file {
    struct af_ops *ops;
};
/*
 * Input attributes for create a writable file.
 * @path    : file path
 * */
struct file_attr {
    char *path;
};

extern struct write_file* create_wfile(struct file_attr *attr);
extern void destroy_wfile(struct write_file *wf);

extern struct read_file* create_rfile(struct file_attr *attr);
extern void destroy_rfile(struct read_file *wf);

extern struct append_file* create_afile(struct file_attr *attr);
extern void destroy_afile(struct append_file *wf);

extern int32_t fs_rdable(const char *path);
extern int32_t fs_wrable(const char *path);
extern int32_t fs_exist(const char *path);
#endif /* INCLUDE_WR_FILE_H_ */
