/* 向epoll中添加/修改和删除被监听的描述符。
 * 并负责为事件分发处理函数 */

#include "Epoll.h"
#include <assert.h>

const int EVENTS_NUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef std::shared_ptr<Channel> Ptochannel;

/* epoll_create1(int flags); same as epoll_create, the unused size parameter
 * has been dropped. */
Epoll::Epoll() : epollfd(epoll_create1(EPOLL_CLOEXEC)), epl_events(EVENTS_NUM) {
    assert(epollfd > 0);
}

Epoll::~Epoll() {}

void Epoll::epoll_add(Ptochannel request, int timeout) {
    int fd = request->getfd();
    if (timeout > 0) {
        add_timer(request, timeout);
        fd2http[fd] = request->getholder();
    }

    struct epoll_event evnt;
    evnt.data.fd = fd;
    evnt.events = request->getevents();

    /* 将当前epoll事件更新 */
    request->equalandupdatelastevents();

    fd2chan[fd] = request;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &evnt) < 0) {
        perror("epoll_add error");
        fd2chan[fd].reset();
    }
}

void Epoll::epoll_mod(Ptochannel request, int timeout) {
    if (timeout > 0)
        add_timer(request, timeout);

    int fd = request->getfd();
    if (!request->equalandupdatelastevents()) {
        struct epoll_event evnt;
        evnt.data.fd = fd;
        evnt.events = request->getevents();
        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &evnt) < 0) {
            perror("epoll_mod error");
            fd2chan[fd].reset();
        }
    }
}

void Epoll::epoll_del(Ptochannel request) {
    int fd = request->getfd();
    struct epoll_event evnt;
    evnt.data.fd = fd;
    evnt.events = request->getevents();
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &evnt) < 0)
        perror("epoll_del error");
    fd2chan[fd].reset();
    fd2http[fd].reset();
}

/* 返回事件 */
std::vector<Ptochannel> Epoll::poll() {
    while (true) {
        int eventcount;
        if ((eventcount = 
        epoll_wait(epollfd, &(*epl_events.begin()), epl_events.size(), EPOLLWAIT_TIME))
         < 0)
            perror("epoll_wait error");
        
        std::vector<Ptochannel> reqst = geteventrequest(eventcount);
        if (reqst.size() > 0)
            return reqst;
    }
}

void Epoll::handleexpired() {
    timermanager.handleexpiredevent();
}

/* 分发处理函数 */
std::vector<Ptochannel> Epoll::geteventrequest(int num) {
    std::vector<Ptochannel> reqst;
    for (int i = 0; i < num; i++) {
        int fd = epl_events[i].data.fd;

        Ptochannel curreq = fd2chan[fd];

        if (curreq) {
            curreq->setrevents(epl_events[i].events);
            curreq->setevents(0);
            reqst.push_back(curreq);
        }
        else
            /*    */ ;
    }
    return reqst;
}

void Epoll::add_timer(Ptochannel request, int timeout) {
    std::shared_ptr<Httpdata> ptohdata = request->getholder();
    if (ptohdata)
        timermanager.addtimer(ptohdata, timeout);
    else
     /*   */;
}