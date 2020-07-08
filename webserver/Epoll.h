/* 向epoll中添加/修改和删除被监听的描述符。
 * 并负责为事件分发处理函数 */

#pragma once
#include "Channel.h"
#include "Httpdata.h"
#include "Timer.h"
#include <memory>
#include <vector>

class Epoll {
    public:
        typedef std::shared_ptr<Channel> Ptochannel;
        
        Epoll();
        ~Epoll();
        
        void epoll_add(Ptochannel request, int timeout);
        void epoll_mod(Ptochannel request, int timeout);
        void epoll_del(Ptochannel request);

        std::vector<Ptochannel> poll();
        std::vector<Ptochannel> geteventrequest(int num);

        void add_timer(Ptochannel request, int timeout);

        int getepollfd() {
            return epollfd;
        }

        void handleexpired();

    private:
        static const int MAX_FD_NUM = 100000;
        int epollfd;
        std::vector<epoll_event> epl_events;
        Ptochannel fd2chan[MAX_FD_NUM];
        std::shared_ptr<Httpdata> fd2http[MAX_FD_NUM];
        Timermanager timermanager;
};