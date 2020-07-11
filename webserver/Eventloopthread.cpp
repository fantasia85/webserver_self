/* EventLoopThread.(事件循环线程，由事件循环线程池管理).
 * 用于管理具体的线程(Thread)和事件循环(Eventloop) */

#pragma once
#include "Eventloopthread.h"
#include <assert.h>

Eventloopthread::Eventloopthread() 
    : loop(NULL),
      exiting(false),
      ethread(std::bind(&Eventloopthread::Threadfunc, this), "Eventloopthread"),
      mutex(),
      cond(mutex)  {}

Eventloopthread::~Eventloopthread() {
    exiting = true;
    if (loop != NULL) {
        loop->quit();
        ethread.join();
    }
}

Eventloop *Eventloopthread::startloop() {
    assert(!ethread.isstarted());
    ethread.start();
    
    {
        Mutexlockguard mlock(mutex);
        /* 确保Threadfunc已经真正运行了 */
        while (loop == NULL)
            cond.wait();
    }

    return loop;
}

void Eventloopthread::Threadfunc() {
    Eventloop eloop;

    {
        Mutexlockguard mlock(mutex);
        loop = &eloop;
        cond.notify();
    }

    eloop.loop();
    loop = NULL;
}