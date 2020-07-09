/* 通过在新定义的类中继承该类，可以将类定义为不可拷贝的 */

#pragma once

class noncopyable {
    protected:
        noncopyable() {}
        ~noncopyable() {}

    private:
        noncopyable(const noncopyable &ncopyobj);
        const noncopyable& operator=(const noncopyable &ncopyobj);
};