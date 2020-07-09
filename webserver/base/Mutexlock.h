/* 定义互斥锁及其加锁和解锁的操作 */

#pragma once 
#include <pthread.h>
/* 继承noncopyable类，将该类声明为不可拷贝的类对象 */
#include "noncopyable.h"

class Mutexlock : noncopyable {
    public:
        Mutexlock() {
            pthread_mutex_init(&mutex, NULL);
        }

        ~Mutexlock() {
            pthread_mutex_lock(&mutex);
            pthread_mutex_destroy(&mutex);
        }

        void lock() {
            pthread_mutex_lock(&mutex);
        }

        void unlock() {
            pthread_mutex_unlock(&mutex);
        }

        pthread_mutex_t *get() {
            return &mutex;
        }

    private:
        pthread_mutex_t mutex;

    /* 友元类，该类为条件变量 */
    private:
        friend class Condition;
};

class Mutexlockguard : noncopyable {
    public:
        explicit Mutexlockguard(Mutexlock &mtxlock) : mutex(mtxlock) {
            mtxlock.lock();
        }
        
        ~Mutexlockguard() {
            mutex.unlock();
        }

    private:
        Mutexlock &mutex;
};