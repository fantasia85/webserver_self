/* Reactor结构中的事件，负责一个文件描述符的IO事件。 
 * 保存IO事件的类型及对应的回调函数。 */

#pragma once
#include <functional>
#include <memory>
#include <sys/epoll.h>

class Eventloop;
class Httpdata;

class Channel {
    private:
        typedef std::function<void()> Callback;
        Eventloop *loop;
        int fd;
        __uint32_t events;
        __uint32_t revents;
        __uint32_t lastevents;

        std::weak_ptr<Httpdata> wptohdata;

        int parse_URI();
        int parse_Headers();
        int analysisrequest();

        Callback readhandler;
        Callback writehandler;
        Callback errorhandler;
        Callback connhandler;

    public:
        Channel(Eventloop *loop);
        Channel(Eventloop *loop, int fd);
        ~Channel();
        int getfd();
        void setfd(int fd);

        void setholder(std::shared_ptr<Httpdata> ptohdata) {
            wptohdata = ptohdata;
        }

        std::shared_ptr<Httpdata> getholder() {
            std::shared_ptr<Httpdata> ptohdata(wptohdata.lock());
            return ptohdata;
        }

        void setreadhandler (Callback &&rdhandler) {
            readhandler = rdhandler;
        }

        void setwritehandler (Callback &&wrhandler) {
            writehandler = wrhandler;
        }

        void seterrorhandler (Callback &&erhandler) {
            errorhandler = erhandler;
        }

        void setconnhandler (Callback && cnnhandler) {
            connhandler = cnnhandler;
        }

        void handlerevents() {
            events = 0;
            if ((revents & EPOLLHUP) && !(revents & EPOLLIN)) {
                events = 0;
                return;
            }
            if (revents & EPOLLERR) {
                if (errorhandler)
                    errorhandler();
                events = 0;
                return;
            }
            
            /* read */
            if (revents & (EPOLLIN | EPOLLPRI || EPOLLRDHUP))
                handleread();
            
            /* write */
            if (revents & EPOLLOUT)
                handlewrite();

            /*  */
            handleconn();
        }

        void handleread();
        void handlewrite();
        void handleerror(int fd, int err_num, std::string err_msg);
        void handleconn();

        void setrevents(__uint32_t evnt) {
            revents = evnt;
        }

        void setevents(__uint32_t evnt) {
            events = evnt;
        }

        __uint32_t &getevents() {
            return events;
        }

        bool equalandupdatelastevents() {
            bool isequal = (lastevents == events);
            lastevents = events;
            return isequal;
        }

        __uint32_t getlastevents() {
            return lastevents;
        }
};