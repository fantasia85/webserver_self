/* EventLoopThread.(事件循环线程，由事件循环线程池管理).
 * 用于管理具体的线程(Thread)和事件循环(Eventloop) */

#pragma once
#include "Eventloop.h"
#include "base/noncopyable.h"
#include "base/Thread.h"
#include "base/Mutexlock.h"
#include "base/Condition.h"

/* 继承noncopyable类，使该类不可以被拷贝构造和拷贝赋值 */
class Eventloopthread : noncopyable {
    public:
        Eventloopthread();
        ~Eventloopthread();
        Eventloop *startloop();

    private:
        void Threadfunc();
        Eventloop *loop;
        bool exiting;
        Thread ethread;
        Mutexlock mutex;
        Condition cond;
};