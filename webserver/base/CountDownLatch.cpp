/* 仿写CountDownLatch. CountDownLatch是一个同步工具类，用来协调多个
 * 线程之间的同步，或者说起到线程之间的通信（而不是用作互斥作用）。 */

#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int cnt) 
    : mutex(), cond(mutex), count(cnt) {}

void CountDownLatch::wait() {
    Mutexlockguard mlock(mutex);
    while (count > 0)
        cond.wait();
}

void CountDownLatch::countdown() {
    Mutexlockguard mlock(mutex);
    --count;
    if (!count)
        cond.notifyall();
}