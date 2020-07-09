/* 线程 */

#include "Thread.h"
#include "Currentthread.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <string>
#include <sys/prctl.h>

/* __thread变量每个线程有一份独立实体，各个线程的值互不干扰 */
namespace CurrentThread {
    __thread int t_cachedtid = 0;
    __thread char t_tidstring[32];
    __thread int t_tidstringlength = 6;
    __thread const char * t_threadname = "default";
}

pid_t gettid() {
    return static_cast<pid_t> (::syscall(SYS_gettid));
}

void Currentthread::cachetid() {
    if (t_cachedtid == 0) {
        t_cachedtid = gettid();
        /* t_tidstringlength = snprintf(t_tidstring, sizeof t_tidstring, "%5d", t_cachedtid); */
    }
}

/* 用以在线程中保留name, tid一类的数据 */
struct Threaddata {
    typedef Thread::Threadfunc Threadfunc;
    Threadfunc func;
    std::string name;
    pid_t *ptotid;
    CountDownLatch *ptolatch;

    Threaddata(const Threadfunc &tfunc, const std::string &namestr, pid_t *ptid, 
                CountDownLatch *platch)
        : func(tfunc), name(namestr), ptotid(ptid), ptolatch(platch) {}

    void runinthread() {
        *ptotid = Currentthread::tid();
        ptotid = NULL;
        ptolatch->countdown();
        ptolatch = NULL;
        Currentthread::t_threadname = name.empty() ? "Thread" : name.c_str();
        /* 给线程命名 */
        prctl(PR_SET_NAME, Currentthread::t_threadname);

        func();
        Currentthread::t_threadname = "finished";
    }
};

