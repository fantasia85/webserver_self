/* EventLoopThreadPool.
 * 事件循环线程池
 *                 */

#pragma once 

#include "base/noncopyable.h"
#include "Eventloopthread.h"

class Eventloopthreadpool : noncopyable {
    public:
        Eventloopthreadpool(Eventloop *evtbaseloop, int evtnumthreads);
        
        ~Eventloopthreadpool() { }

        void start();

        Eventloop *getnextloop();

    private:
        Eventloop *baseloop;
        bool started;
        int numthreads;
        int next;
        std::vector<std::shared_ptr<Eventloopthread>> vthreads;
        std::vector<Eventloop *> loops;
};