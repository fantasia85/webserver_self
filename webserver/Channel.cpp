/* Reactor结构中的事件，负责一个文件描述符的IO事件。 
 * 保存IO事件的类型及对应的回调函数。 */

#include "Channel.h"

Channel::Channel(Eventloop *evntloop) 
    : loop(evntloop), fd(0), events(0), lastevents(0)  {}

Channel::Channel(Eventloop *evntloop, int fdtor) 
    : loop(evntloop), fd(fdtor), events(0), lastevents(0) {}

Channel::~Channel() {}

int Channel::getfd() {
    return  fd;
}

void Channel::setfd(int fdtor) {
    fd = fdtor;
}

void Channel::handleread() {
    if (readhandler)
        readhandler();
}

void Channel::handlewrite() {
    if (writehandler)
        writehandler();
}

void Channel::handleconn() {
    if (connhandler)
        connhandler();
}
