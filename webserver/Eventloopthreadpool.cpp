/* EventLoopThreadPool.
 * 事件循环线程池
 *                 */

#include "Eventloopthreadpool.h"
#include <assert.h>

Eventloopthreadpool::Eventloopthreadpool(Eventloop *evtbaseloop, int evtnumthreads)
    : baseloop(evtbaseloop), started(false), numthreads(evtnumthreads), next(0) {
        if (numthreads <= 0) {
            /*  */
            abort();
        }
}

void Eventloopthreadpool::start() {
    assert(baseloop->isinloopthread());
    started = true;
    for (int i = 0; i < numthreads; i++) {
        std::shared_ptr<Eventloopthread> ptoe(new Eventloopthread());
        vthreads.push_back(ptoe);
        loops.push_back(ptoe->startloop());
    }
}

Eventloop *Eventloopthreadpool::getnextloop() {
    assert(baseloop->isinloopthread());
    assert(started);
    Eventloop *loop = baseloop;
    if (!loops.empty()) {
        loop = loops[next];
        next = (next + 1) % numthreads;
    }
    return loop;
}