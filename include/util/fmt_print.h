/*
 * File Name: fmt_print.h
 *
 * Created on: 2015年9月24日
 *     Author: sharpllr@163.com
 * Modify records:
 *  -20151019 lirui.Liu
 *      |--syslog support       (add)
 */

#ifndef CM_INC_FMT_PRINT_H_
#define CM_INC_FMT_PRINT_H_

#ifdef _SYS_LOG_
#define SYS_LOG_FACILITY            LOG_LOCAL0
#endif

#ifdef _TERM_PRT_
#include <stdio.h>
#define prt_dir(format, args...) \
do{\
    printf("[%s][%s][%d]"format"\n", __FILE__, __func__, __LINE__, ##args); \
}while(0)
#define logEasy(format, args...) \
do{\
    prt_dir(format, ##args);\
}while(0)

#define logTrace(format, args...) \
do{\
    prt_dir("[TRACE]"format, ##args);\
}while(0)


#define logWarn(format, args...) \
do{\
    prt_dir("[WARN]"format, ##args);\
}while(0)


#define logInfo(format, args...) \
do{\
    prt_dir("[INFO]"format, ##args);\
}while(0)

#define logErr(format, args...) \
do{\
    prt_dir("[ERROR]"format, ##args);\
}while(0)

#elif defined (_SYS_LOG_)
#include <syslog.h>
#define prt_dir(lvl, format, args...) \
do{\
    syslog(lvl, "[%s][%s][%d]"format"\n", __FILE__, __func__, __LINE__, ##args);\
}while(0)
#define logTrace(format, args...) \
do{\
    prt_dir(LOG_DEBUG, format, ##args);\
}while(0)

#define logInfo(format, args...) \
do{\
    prt_dir(LOG_INFO, format, ##args);\
}while(0)

#define logErr(format, args...) \
do{\
    prt_dir(LOG_ERR, format, ##args);\
}while(0)
#endif


#ifdef _SYS_LOG_
static inline int init_fmt_print()
{
    int ret = 0;

    /*
     * Need configure /etc/rsyslog.conf to specify local0.* file.
     * And restart rsyslog service.
     * */
    openlog(NULL, LOG_CONS|LOG_PID|LOG_NDELAY, SYS_LOG_FACILITY);
    return ret;
}
#endif

#endif /* CM_INC_FMT_PRINT_H_ */
