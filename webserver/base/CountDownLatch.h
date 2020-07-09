/* 仿写CountDownLatch. CountDownLatch是一个同步工具类，用来协调多个
 * 线程之间的同步，或者说起到线程之间的通信（而不是用作互斥作用）。
 * 基本概念：
 * CoountDownLatch能够使一个线程在等待另外一些线程完成各自的工作后，
 * 在继续执行。使用一个计数器实现。当一个线程完成自己的任务后，计数器
 * 的值就会减1。当计数器的值为0时，表示所有的线程都已经完成了一些任
 * 务，等待的线程就可以恢复执行接下来的任务 */

#pragma once

#include "Condition.h"
#include "Mutexlock.h"
#include "noncopyable.h"

class CountDownLatch : noncopyable {
    public:
        explicit CountDownLatch (int count);
        void wait();
        void countdown();

    private:
        mutable Mutexlock mutex;
        Condition cond;
        int count;
};