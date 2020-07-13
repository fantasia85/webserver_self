/* 定时器，用以处理超时请求和长期不活跃的连接 */

#pragma once
#include <memory>
#include <deque>
#include <queue>
#include "base/Currentthread.h"

class Httpdata;

class TimerNode {
    public:
        TimerNode(std::shared_ptr<Httpdata> requestdata, int timeout);
        ~TimerNode();
        TimerNode (TimerNode &tnode);
        
        void updata(int timeout);
        bool isvalid();
        void clearrequest();
        
        void setdeleted() {
            deleted_ = true;
        }
        
        bool isdeleted() const {
            return deleted_;
        }

        size_t getexpiredtime() const {
            return expiredtime_;
        }

    private:
        bool deleted_;
        size_t expiredtime_;
        std::shared_ptr<Httpdata> ptohttpdata;
};

/* functors (function objects) 
 * 用于构建小顶堆 */
struct timercmp {
    bool operator() (std::shared_ptr<TimerNode> &tnodea, std::shared_ptr<TimerNode> &tnodeb) const {
        return tnodea->getexpiredtime() > tnodeb->getexpiredtime();
    }
};

class Timermanager {
    public:
        Timermanager();
        ~Timermanager();

        void addtimer(std::shared_ptr<Httpdata> ptohttpdata, int timeout);
        void handleexpiredevent();

    private:
        typedef std::shared_ptr<TimerNode> Ptotimernode;
        std::priority_queue<Ptotimernode, std::deque<Ptotimernode>, timercmp> timernodequeue;
};