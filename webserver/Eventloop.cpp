/* per thread one loop.
 * 使用epoll等待事件，事件发生后处理事件。同时可以利用空闲
 * 的I/O线程执行额外的计算任务 */

#include "Eventloop.h"
#include <sys/eventfd.h>
#include "tcpfunc.h"
#include <assert.h>

__thread Eventloop *t_loopinthisthread = 0;

/* 创建eventfd */
int createeventfd() {
    int evntfd;
    if ((evntfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) < 0) {
        /*  */
        abort();
    }
    return evntfd;
}

Eventloop::Eventloop ()
    : looping(false),
      poller(new Epoll()),
      wakeupfd(createeventfd()),
      isquit(false),
      eventhandling(false),
      callingpendingfunctors(false),
      threadid(Currentthread::tid()),
      ptowakeupchannel(new Channel(this, wakeupfd)) {
    
    if (t_loopinthisthread) {
        /*  */
    }
    else {
        t_loopinthisthread = this;
    }
    /* 设置为ET模式 */
    ptowakeupchannel->setevents(EPOLLIN | EPOLLET);
    ptowakeupchannel->setreadhandler(std::bind(&Eventloop::handleread, this));
    ptowakeupchannel->setconnhandler(std::bind(&Eventloop::handleconn, this));
    poller->epoll_add(ptowakeupchannel, 0);
}

void Eventloop::handleconn() {
    updatepoller(ptowakeupchannel, 0);
}

Eventloop::~Eventloop() {
    close(wakeupfd);
    t_loopinthisthread = NULL;
}

/* 利用 eventfd 唤醒 Eventloop */
void Eventloop::wakeup() {
    /* eventfd 必须读取8字节的整型变量 */
    uint64_t val = 1;
    ssize_t n = writen(wakeupfd, (char *)(&val), sizeof(val));
    if (n != sizeof(val)) {
        /*   */
    }
}

/* eventfd的读回调函数 */
void Eventloop::handleread() {
    uint64_t val = 1;
    ssize_t n = readn(wakeupfd, &val, sizeof(val));
    if (n != sizeof(val)) {
        /*  */
    }
    ptowakeupchannel->setevents(EPOLLIN | EPOLLET);
}

/* 在I/O线程中调用某个函数 */
void Eventloop::runinloop(Functor &&cb) {
    /* 在当前I/O线程中调用，同步调用cb函数 */
    if (isinloopthread())
        cb();
    else
        /* 否则异步添加到任务队列中 */ 
        queueinloop(std::move(cb));
}

/* 将任务添加到任务队列中 */
void Eventloop::queueinloop(Functor &&cb) {
    {
        Mutexlockguard mlock(mutex);
        pendingfunctors.emplace_back(std::move(cb));
    }

    if (!isinloopthread() || callingpendingfunctors)
        wakeup();
}

/* 事件循环，不能跨越线程调用，只能在创建该对象的线程中调用 */
void Eventloop::loop() {
    assert(!looping);
    assert(isinloopthread());
    looping = true;
    isquit = false;
    std::vector<std::shared_ptr<Channel>> ret;
    while (!isquit) {
        ret.clear();
        ret = poller->poll();
        eventhandling  = true;
        for (auto &it : ret)
            it->handlerevents();
        eventhandling = false;
        /* 当I/O线程空闲时，使用该线程进行额外的计算任务 */
        dopendingfunctors();
        poller->handleexpired();
    } 

    looping = false;
}

void Eventloop::dopendingfunctors() {
    std::vector<Functor> functors;
    callingpendingfunctors = true;

    {
        Mutexlockguard mlock(mutex);
        functors.swap(pendingfunctors);
    }

    for (size_t i = 0; i < functors.size(); i++) {
        functors[i]();
    }
    
    callingpendingfunctors = false;
}

void Eventloop::quit() {
    isquit = true;
    if (!isinloopthread()) {
        wakeup();
    }
}