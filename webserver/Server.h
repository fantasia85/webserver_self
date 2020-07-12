/* server. Create acceptor and eventloopthreadpool. */

#pragma once
#include "Channel.h"
#include "Eventloop.h"
#include "Eventloopthreadpool.h"

class Server {
    public:
        Server(Eventloop *eloop, int threadn, int prt);
        ~Server() {}

        Eventloop *getloop() const {
            return loop;
        }

        void start();
        void handnewconn();

        void handthisconn() {
            loop->updatepoller(acceptor);
        }

    private:
        Eventloop *loop;
        int threadnum;
        std::unique_ptr<Eventloopthreadpool> evtloopthreadpool;
        bool started;
        std::shared_ptr<Channel> acceptor;
        int port;
        int listenfd;
        static const int MAX_FD = 100000;
};