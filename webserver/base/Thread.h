/* 线程 */

#pragma once

#include "CountDownLatch.h"
#include "noncopyable.h"
#include <pthread.h>
#include <functional>
#include <string>

class Thread : noncopyable {
    public:
        typedef std::function<void()> Threadfunc;
        explicit Thread(const Threadfunc &, const std::string &namestring = std::string());
        ~Thread();

        void start();
        int join();
        
        bool isstarted() const {
            return started;
        }

        pid_t getid() const {
            return pid;
        }

        const std::string &name() const {
            return namestr;
        }

    private:
        void setdefaultname();
        bool started;
        bool joined;
        pthread_t pthreadid;
        pid_t pid;
        Threadfunc func;
        std::string namestr;
        CountDownLatch latch;
};