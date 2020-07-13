/* 定时器，用以处理超时请求和长期不活跃的连接 */

#include "Timer.h"
#include <sys/time.h>

TimerNode::TimerNode(std::shared_ptr<Httpdata> requestdata, int timeout) 
    : deleted_(false), ptohttpdata(requestdata) {
    struct timeval now;
    gettimeofday(&now, NULL);

    expiredtime_ = ((now.tv_sec % 10000) * 1000 + now.tv_usec / 1000)
                    + timeout;        
}

TimerNode::~TimerNode() {
    /*if (ptohttpdata)
        ptohttpdata->handleclose(); */
}

TimerNode::TimerNode(TimerNode &tn) 
    : expiredtime_(0), ptohttpdata(tn.ptohttpdata)  {}

void TimerNode::updata(int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredtime_ = ((now.tv_sec % 10000) * 1000 + now.tv_usec / 1000)
                    + timeout;
}

bool TimerNode::isvalid() {
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t curtime = ((now.tv_sec % 10000) * 1000 + now.tv_usec / 1000);
    if (curtime < expiredtime_)
        return true;
    else {
         this->setdeleted(); /* set deleted_ true and return false */
         return false;
    }
}

void TimerNode::clearrequest() {
    ptohttpdata.reset();
    this->setdeleted();
}

Timermanager::Timermanager() {}

Timermanager::~Timermanager() {}

void Timermanager::addtimer(std::shared_ptr<Httpdata> ptohttpdata, int timeout) {
    Ptotimernode ptr_timernode(new TimerNode(ptohttpdata, timeout));
    timernodequeue.push(ptr_timernode);
    /*
    ptohttpdata->linkTimer(ptr_timernode);
    */
}

/*
 * 采用优先队列。当某结点被置为deleted之后，并不会马上删除。
 * 而是知道超时或者它前面的节点都被删除之后，才会删除该节点 */
void Timermanager::handleexpiredevent() {
    while (!timernodequeue.empty()) {
        Ptotimernode ptr_timernode = timernodequeue.top();
        if (ptr_timernode->isdeleted())
            timernodequeue.pop();
        else if (!ptr_timernode->isvalid())
            timernodequeue.pop();
        else
            break;
    }
}