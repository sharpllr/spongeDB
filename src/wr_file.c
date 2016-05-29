/*
 * wr_file.c
 *
 *  Created on: 2016年2月7日
 *      Author: liulirui
 */

#include <errno.h>
#include <fcntl.h>
#include "sponge.h"
#include "util/spg_util.h"
#include "util/fmt_print.h"
#include "wr_file.h"

/*
 * Private structure for writable file
 * @wfile   : writable file operations.
 * @file    : FILE pointer of writable file.
 * */
struct wf_struct {
    struct write_file wfile;
    FILE *file;
    char *path;
    int32_t fd;
};

//Private structure for readable file
struct rf_struct {
    struct read_file rfile;
    FILE *file;
    char *path;
    int32_t fd;
};

typedef struct wf_struct af_struct_st;

FILE* file_open(const char *path, const char *mode)
{
    FILE *fl;
    if (0 != access(path, F_OK)) {
        fl = fopen(path, "w");
        fclose(fl);
    }

    fl = fopen(path, mode); //for fseek
    if (NULL == fl) {
        logInfo("Open a file stream failed.");
        return NULL;
    }

    return fl;
}

static inline int32_t file_close(FILE *fl)
{
    return fclose(fl);
}

static inline int32_t file_unlock(int32_t fd)
{
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_pid  = getpid();
    return fcntl(fd, F_SETLK, &lock);
}

static inline int32_t bias_adjust(FILE *file, int64_t off)
{
    int32_t ret;
    ret = (off < 0)?fseek(file, (long)(off+1), SEEK_END):
        fseek(file, (long)off, SEEK_SET);
    return (-1 == ret)?SPG_ERR:SPG_OK;
}

/*
 * @offset:
 *   0: begin of file
 *  <0: offset to end of file,including EOF
 *  >0: offset from beginning of file
 *
 * return
 *  -1: error
 *  0 : success
 * */
int32_t _wf_write(struct write_file *file, int64_t offset, void *data, size_t len)
{
    int32_t ret;
    size_t wrLen;
    struct wf_struct *wf = (struct wf_struct*)file;
    
    if (0 != access(wf->path, F_OK) || -1 == wf->fd || NULL == wf->file) {
        return -1;
    }

    if (offset < 0) {
        ret = fseek(wf->file, (long)(offset+1), SEEK_END);
    } if (offset >= 0) {
        ret = fseek(wf->file, (long)offset, SEEK_SET);
    }
    if (-1 == ret) {
        logInfo("File seek failed,error code=0x%x.", errno);
        return SPG_ERR;
    }

    wrLen = fwrite(data, 1, len, wf->file);
    if (wrLen != len) {
        logInfo("File write error,%zd byte wrote.", wrLen);
        return SPG_ERR;
    }

    return SPG_OK;
}


int32_t _wf_open(struct write_file *file)
{
    struct wf_struct *wf = (struct wf_struct*)file;
    wf->file = file_open(wf->path, "rb+");
    if (NULL == wf->file) {
        return SPG_ERR;
    }
    wf->fd   = fileno(wf->file);
    return SPG_OK;
}

int32_t _wf_close(struct write_file *file) 
{
    int32_t ret;
    struct wf_struct *wf = (struct wf_struct*)file;
    if (-1 != wf->fd) {
        ret = file_close(wf->file);
        if (EOF == ret) {
            logInfo("Exception when close file,error code=0x%x.", errno);
        }
        wf->file = NULL;
        wf->fd   = -1;
    }
    return SPG_OK;
}

int32_t _wf_flush(struct write_file *file)
{
    struct wf_struct *wf = (struct wf_struct*)file;
    if (wf->fd == -1) {
        logErr("Parameter error.");
        return SPG_ERR;
    }

    if (EOF == fflush(wf->file)) {
        logInfo("Flush error.error code = 0x%x.", errno);
        return SPG_ERR;
    }

    return SPG_OK;
}

void _wf_wrlock(struct write_file *file)
{
    struct wf_struct *wf = (struct wf_struct*)file;
    if (wf->fd == -1) {
        logErr("Parameter error.");
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_pid  = getpid();

    if (fcntl(wf->fd, F_SETLKW, &lock)) {
        logInfo("Set write lock failed,error code=0x%x.", errno);
    }
}

int32_t _wf_try_wrlock(struct write_file *file)
{
    struct wf_struct *wf = (struct wf_struct*)file;
    if (wf->fd == -1) {
        logErr("Parameter error.");
        return SPG_ERR;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_pid  = getpid();

    if (-1 == fcntl(wf->fd, F_SETLK, &lock)) {
#if _CHECK_ERR_DETAIL_
        if (errno == EACCES || errno == EAGAIN) {   //lock occupied
            logTrace("File is occupied by others.");
            return WRFILE_RET_LOCK_OCCUPIED;
        }
        else {
            logInfo("Acquire file lock failed,error code=0x%x.", errno);
            return SPG_ERR;
        }
#endif
        return SPG_ERR;
    }

    return SPG_OK;
}

void    _wf_unlock(struct write_file *file)
{
    int32_t ret;
    struct wf_struct *wf = (struct wf_struct*)file;
    if (wf->fd == -1) {
        logErr("Parameter error.");
        return ;
    }
    
    ret = file_unlock(wf->fd);
    if (-1 == ret) {
        logInfo("Unlock file failed.error code=0x%x", errno);
    }
}

int32_t _rf_open(struct read_file *file)
{
    struct rf_struct *rf = (struct rf_struct*)file;

    rf->file = file_open(rf->path, "r");
    if (NULL == rf->file) {
        return SPG_ERR;
    }
    rf->fd   = fileno(rf->file);

    return SPG_OK;
}

int32_t _rf_close(struct read_file *file)
{
    int32_t ret;
    struct rf_struct *rf = (struct rf_struct*)file;

    ret = file_close(rf->file); //If fclose failed,the FILE* is disabled
    if (EOF == ret) {
        logInfo("Close readable file failed.error code=0x%x.", errno);
    }
    rf->file = NULL;
    rf->fd   = -1;

    return SPG_OK;
}

size_t _rf_bias_read(struct read_file *file, int64_t offset,
    void *buf, size_t len)
{
    int32_t ret;
    size_t rdLen = 0;
    struct rf_struct *rf = (struct rf_struct*)file;

    if (0 != access(rf->path, F_OK) || rf->fd == -1) {
        return 0;
    }

    if (SPG_OK != bias_adjust(rf->file, offset)) {
        return SPG_ERR;
    }

    rdLen = fread(buf, 1, len, rf->file);
    if (rdLen == len) {
        return len;
    }
#if _CHECK_ERR_DETAIL_
    ret = feof(rf->file);
    if (ret) {
        clearerr(rf->file);
        return rdLen;
    }

    ret = ferror(rf->file);
    if (ret) {
        clearerr(rf->file);
        logInfo("Read file failed.error code=0x%x.", errno);
        dbg_assert(0);
    }
#endif

    return rdLen;
}

size_t _rf_read(struct read_file *file, void *buf, size_t len)
{
    int32_t ret;
    size_t rdLen = 0;
    struct rf_struct *rf = (struct rf_struct*)file;

    if (0 != access(rf->path, F_OK) || rf->fd == -1) {
        return 0;
    }

    rdLen = fread(buf, 1, len, rf->file);
    if (rdLen == len) {
        return len;
    }
#if _CHECK_ERR_DETAIL_
    ret = feof(rf->file);
    if (ret) {
        clearerr(rf->file);
        return rdLen;
    }

    ret = ferror(rf->file);
    if (ret) {
        clearerr(rf->file);
        logInfo("Read file failed.error code=0x%x.", errno);
        dbg_assert(0);
        return 0;
    }
#endif

    return rdLen;
}

void _rf_rdlock(struct read_file *file)
{
    struct rf_struct *rf = (struct rf_struct*)file;
    if (rf->fd == -1) {
        logErr("Parameter error.");
        return;
    }

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_pid  = getpid();

    if (fcntl(rf->fd, F_SETLKW, &lock)) {
        logInfo("Set read lock failed,error code=0x%x.", errno);
    }
}

int32_t _rf_try_rdlock(struct read_file *file)
{
    struct rf_struct *rf = (struct rf_struct*)file;
    if (rf->fd == -1) {
        logErr("Parameter error.");
        return SPG_ERR;
    }

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_pid  = getpid();

    if (-1 == fcntl(rf->fd, F_SETLK, &lock)) {
#if _CHECK_ERR_DETAIL_
        if (errno == EACCES || errno == EAGAIN) {   //lock occupied
            logTrace("File is occupied by others.");
            return WRFILE_RET_LOCK_OCCUPIED;
        }
        else {
            logInfo("Acquire file lock failed,error code=0x%x.", errno);
            return SPG_ERR;
        }
#endif
        return SPG_ERR;
    }

    return SPG_OK;
}

void _rf_unlock(struct read_file *file)
{
    int32_t ret;
    struct rf_struct *rf = (struct rf_struct*)file;
    if (rf->fd == -1) {
        logErr("Parameter error.");
        return ;
    }

    ret = file_unlock(rf->fd);
    if (-1 == ret) {
        logInfo("Unlock file failed.error code=0x%x", errno);
    }
}

int32_t _af_append(struct append_file *file, void *data, size_t len)
{
    size_t wrLen;
    struct wf_struct *wf = (struct wf_struct*)file;

    if (0 != access(wf->path, F_OK) || -1 == wf->fd || NULL == wf->file) {
        return SPG_ERR;
    }

    wrLen = fwrite(data, 1, len, wf->file);
    if (wrLen != len) {
        logInfo("File write error,%zd byte wrote.", wrLen);
        return SPG_ERR;
    }

    return SPG_OK;
}

int32_t _af_open(struct append_file *file)
{
    struct wf_struct *wf = (struct wf_struct*)file;
    wf->file = file_open(wf->path, "a+");
    if (NULL == wf->file) {
        return SPG_ERR;
    }
    wf->fd   = fileno(wf->file);
    return SPG_OK;
}

static inline int32_t _af_close(struct append_file *file)
{
    return _wf_close((struct write_file*)file);
}

static inline int32_t _af_flush(struct append_file *file)
{
    return _wf_flush((struct write_file*)file);
}

static inline void    _af_aplock(struct append_file *file)
{
    return _wf_wrlock((struct write_file*)file);
}

static inline int32_t _af_try_aplock(struct append_file *file)
{
    return _wf_try_wrlock((struct write_file*)file);
}

static inline void    _af_unlock(struct append_file *file)
{
    return _wf_unlock((struct write_file*)file);
}


struct wf_ops g_default_wf_ops = {
    ._write      = _wf_write,
    ._open       = _wf_open,
    ._close      = _wf_close,
    ._flush      = _wf_flush,
    ._wrlock     = _wf_wrlock,
    ._try_wrlock = _wf_try_wrlock,
    ._unlock     = _wf_unlock
};

struct rf_ops g_default_rf_ops = {
    ._open       = _rf_open,
    ._close      = _rf_close,
    ._bias_read  = _rf_bias_read,
    ._read       = _rf_read,
    ._rdlock     = _rf_rdlock,
    ._try_rdlock = _rf_try_rdlock,
    ._unlock     = _rf_unlock
};

struct af_ops g_default_af_ops = {
    ._open       = _af_open,
    ._append     = _af_append,
    ._flush      = _af_flush,
    ._close      = _af_close,
    ._aplock     = _af_aplock,
    ._try_aplock = _af_try_aplock,
    ._unlock     = _af_unlock
};

struct write_file* create_wfile(struct file_attr *attr)
{
    if (NULL == attr || NULL == attr->path) {
        return NULL;
    }

    struct wf_struct *wf = NULL;
    wf = scalloc(sizeof(*wf));
    if (NULL == wf) {
        logErr("smalloc failed.");
        return NULL;
    }

    wf->path = sstrdup(attr->path);
    dbg_assert(NULL != wf->path);
    if (NULL == wf->path) {
        sfree(wf);
        return NULL;
    }

    wf->wfile.ops = &g_default_wf_ops;

    return &wf->wfile;
}

void destroy_wfile(struct write_file *wf)
{
    struct wf_struct *wfs = (struct wf_struct*)wf;

    sfree(wfs->path);
    if (wfs->fd != -1) {
        fclose(wfs->file);
        wfs->file = NULL;
        wfs->fd   = -1;
    }
    sfree(wfs);
}

struct read_file* create_rfile(struct file_attr *attr)
{
    if (NULL == attr || NULL == attr->path) {
        return NULL;
    }

    struct rf_struct *rf = NULL;
    rf = scalloc(sizeof(*rf));
    if (NULL == rf) {
        logErr("smalloc failed.");
        return NULL;
    }

    rf->path = sstrdup(attr->path);
    dbg_assert(NULL != rf->path);
    if (NULL == rf->path) {
        sfree(rf);
        return NULL;
    }

    rf->rfile.ops = &g_default_rf_ops;

    return &rf->rfile;
}

void destroy_rfile(struct read_file *rf)
{
    struct rf_struct *rfs = (struct rf_struct*)rf;

    sfree(rfs->path);
    if (rfs->fd != -1) {
        fclose(rfs->file);
        rfs->file = NULL;
        rfs->fd   = -1;
    }
    sfree(rfs);
}


struct append_file* create_afile(struct file_attr *attr)
{
    struct append_file *afOps = NULL;
    af_struct_st *af = (af_struct_st*)create_wfile(attr);
    if (NULL == af) {
        return NULL;
    }

    afOps = (struct append_file*)(&af->wfile);
    afOps->ops = &g_default_af_ops;
    return afOps;
}

void destroy_afile(struct append_file *af)
{
    destroy_wfile((void*)af);
}


int32_t fs_rdable(const char *path)
{
    return access(path, R_OK);
}

int32_t fs_wrable(const char *path)
{
    return access(path, W_OK);
}

int32_t fs_exist(const char *path)
{
    return access(path, F_OK);
}
