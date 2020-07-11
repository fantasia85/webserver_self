/* per thread one loop.
 * 使用epoll等待事件，事件发生后处理事件。同时可以利用空闲
 * 的I/O线程执行额外的计算任务 */

#pragma once
#include <memory>
#include "Epoll.h"
#include "base/Mutexlock.h"
#include <functional>
#include "Channel.h"
#include "base/Currentthread.h"
#include <sys/socket.h>
#include <vector>

class Eventloop {
    public:
        typedef std::function<void ()> Functor;

        Eventloop();
        ~Eventloop();

        void loop();
        void quit();
        void runinloop(Functor &&cb);
        void queueinloop(Functor &&cb);
        
        bool isinloopthread() const {
            return threadid == Currentthread::tid();
        }

        void Shutdown(std::shared_ptr<Channel> ptochannel) {
            shutdown(ptochannel->getfd(), SHUT_WR);
        }

        void removefrompoller(std::shared_ptr<Channel> ptochannel) {
            poller->epoll_del(ptochannel);
        }

        void updatepoller(std::shared_ptr<Channel> ptochannel, int timeout = 0) {
            poller->epoll_mod(ptochannel, timeout);
        }

        void addtopoller(std::shared_ptr<Channel> ptochannel, int timeout = 0) {
            poller->epoll_add(ptochannel, timeout);
        }

    private:
        bool looping;
        std::shared_ptr<Epoll> poller;
        int wakeupfd;
        bool isquit;
        bool eventhandling;
        mutable Mutexlock mutex;
        std::vector<Functor> pendingfunctors;
        bool callingpendingfunctors;
        const pid_t threadid;
        std::shared_ptr<Channel> ptowakeupchannel;

        void wakeup();
        void handleread();
        void dopendingfunctors();
        void handleconn();
};