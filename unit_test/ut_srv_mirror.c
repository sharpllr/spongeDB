/*
 * ut_srv_mirror.c
 *
 * Author: Lirui.Liu
 *         sharpllr@163.com
 *
 */

#ifndef _UT_CLI_
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include "ae.h"
#include "anet.h"
#include "mem/zmalloc.h"
#include "ut.h"

struct srv_attr {
    int32_t port;       //listen port
    int32_t tcp_backlog;//backlog for listen()
};

struct ae_ops {
    aeFileProc *accptConHandle;
    aeFileProc *handleRmtMsg;
};

struct mr_srv {
    struct srv_attr attr;
    int32_t ipFd[32];   //bind ip
    uint32_t ipFdCnt;   //bind ip count
    aeEventLoop *evl;    //Event loop
    struct ae_ops *aeOps;
};


void listenPort(struct mr_srv *srv)
{
    char errMsg[256];

    //Listen sockfd
    srv->ipFd[srv->ipFdCnt] = anetTcpServer(errMsg, srv->attr.port, NULL,
        srv->attr.tcp_backlog);
    logEasy("Listen fd:%d.", srv->ipFd[srv->ipFdCnt]);
    //set nonblock attr
    anetNonBlock(errMsg, srv->ipFd[srv->ipFdCnt]);
    srv->ipFdCnt++;

    return;
}

void initSrv(struct mr_srv **srv, struct srv_attr *attr,
    struct ae_ops *ops)
{
    dbg_assert(NULL != attr && NULL != ops);

    *srv = zcalloc(sizeof(struct mr_srv));

    (*srv)->aeOps = ops;

    memcpy(&((*srv)->attr), attr, sizeof(*attr));
    (*srv)->ipFdCnt = 0;
    (*srv)->evl = aeCreateEventLoop(16);
    return;
}

/*Test main*/
void acptConnHandle(struct aeEventLoop *eventLoop, int fd, void *clientData,
    int mask);
void handleRmtMsg(struct aeEventLoop *eventLoop, int fd, void *clientData,
    int mask);

struct srv_attr g_attr = {
    .port = 7001,
    .tcp_backlog = 511,
};

struct ae_ops g_srvAeOps = {
    .accptConHandle = acptConnHandle,
    .handleRmtMsg   = handleRmtMsg,
};

struct mr_srv *g_mr_srv = NULL;

void handleRmtMsg(struct aeEventLoop *eventLoop, int fd, void *clientData,
    int mask)
{
    int32_t rdLen;
    char rdData[128];

    rdLen = read(fd, rdData, sizeof(rdData));
    if (rdLen == -1) {
        logErr("Read data failed.errno = %d.", errno);
        dbg_assert(rdLen != -1);
        return;
    }
    else if (rdLen == 0) {
        logErr("Connection lost.");
        close(fd);
        aeDeleteFileEvent(eventLoop, fd, AE_READABLE);
        return;
    }
    logEasy("Read %d byte data.", rdLen);
    logEasy("Read data:%s.", rdData);
}

void handleRemote(struct aeEventLoop *evl, int32_t fd, void *data)
{
    struct mr_srv *srv = (struct mr_srv*)data;
    aeCreateFileEvent(evl, fd, AE_READABLE, srv->aeOps->handleRmtMsg, data);
}

void acptConnHandle(struct aeEventLoop *eventLoop, int fd, void *clientData,
    int mask)
{
    int32_t port;
    int32_t rFd;    //Remote connection file descriptor
    char rIp[46];   //Take IPV6 into consider[46Byte].

    rFd = anetTcpAccept(NULL, fd, rIp, sizeof(rIp), &port);
    if (ANET_ERR == rFd) {
        logEasy("Accept remote failed.");
        dbg_assert(ANET_ERR != rFd);
        return;
    }

    anetKeepAlive(NULL, rFd, 1);
    anetNonBlock(NULL, rFd);
    anetEnableTcpNoDelay(NULL, rFd);

    logEasy("Remote ip = %s.", rIp);
    //Create event handler to receive remote data
    handleRemote(eventLoop, rFd, clientData);

    return;
}


void launch_srv(void)
{
    int idx;

    initSrv(&g_mr_srv, &g_attr, &g_srvAeOps);

    listenPort(g_mr_srv);

    for (idx = 0; idx < g_mr_srv->ipFdCnt; idx++) {
        //Create file for accept connect require
        aeCreateFileEvent(g_mr_srv->evl, g_mr_srv->ipFd[idx], AE_READABLE,
            g_mr_srv->aeOps->accptConHandle, g_mr_srv);
    }

    aeMain(g_mr_srv->evl);
}
#endif
