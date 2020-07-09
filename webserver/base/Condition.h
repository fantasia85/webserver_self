/* 定义条件变量的类，允许多个线程以无竞争的方式等待特定的条件发生 */

#pragma once 

#include "noncopyable.h"
#include "Mutexlock.h"
#include <pthread.h>
#include <time.h>
#include <errno.h>

class Condition : noncopyable {
    public:
    explicit Condition(Mutexlock &mtxlock) : mutex(mtxlock) {
        pthread_cond_init(&cond, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&cond);
    }

    void wait() {
        pthread_cond_wait(&cond, mutex.get());
    }

    /* 至少唤醒一个等待该条件的线程 */
    void notify() {
        pthread_cond_signal(&cond);
    }

    /* 唤醒等待该条件的所有线程 */
    void notifyall() {
        pthread_cond_broadcast(&cond);
    }

    /* timedwait等待的时间值是一个绝对数 */
    bool waitforseconds(int seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        /* ETIMEDOUT位于头文件<errno.h>中 */
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }

    private:
        Mutexlock &mutex;
        pthread_cond_t cond;
};