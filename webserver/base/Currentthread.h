/* 当前线程的信息 */

#pragma once
#include <stdint.h>

namespace Currentthread {
    extern __thread int t_cachedtid;
    // extern __thread char t_tidstring[32]; 
    // extern __thread int t_tidstringlength; 
    extern __thread const char *t_threadname;

    void cachetid();

    inline int tid() {
        if (__builtin_expect(t_cachedtid == 0, 0)) {
            cachetid();
        }
        return t_cachedtid;
    }

    inline const char *name() {
        return t_threadname;
    }
}