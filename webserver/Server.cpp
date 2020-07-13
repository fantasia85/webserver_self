/* server. Create acceptor and eventloopthreadpool. */

#include "Server.h"
#include "tcpfunc.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

Server::Server(Eventloop *eloop, int threadn, int prt) 
    : loop(eloop),
      threadnum(threadn),
      evtloopthreadpool(new Eventloopthreadpool(loop, threadnum)),
      started(false),
      acceptor(new Channel(loop)),
      port(prt),
      listenfd(tcp_listen(port))  {
          acceptor->setfd(listenfd);
          handle_sigpipe();
          if (setsocketnonblocking(listenfd) < 0) {
              perror("set socket nonblock error");
              abort();
          }
}

void Server::start() {
    evtloopthreadpool->start();
    /* 设置为ET模式 */
    acceptor->setevents(EPOLLIN | EPOLLET);
    acceptor->setreadhandler(std::bind(&Server::handnewconn, this));
    acceptor->setconnhandler(std::bind(&Server::handthisconn, this));
    loop->addtopoller(acceptor, 0);
    started = true;
}

void Server::handnewconn() {
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t client_addrlen = sizeof(client_addr);
    int acceptfd = 0;
    while ((acceptfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addrlen)) > 0) {
        Eventloop *eloop = evtloopthreadpool->getnextloop();
        /*   */

        /* 限制最大并发连接数 */
        if (acceptfd >= MAX_FD) {
            close(acceptfd);
            continue;
        }

        /* 设置为非阻塞模式 */
        if (setsocketnonblocking(acceptfd) < 0) {
            /*  */
            return;
        }

        setsocketnodelay(acceptfd);

        std::shared_ptr<Httpdata> reqinfo(new Httpdata(eloop, acceptfd));
        reqinfo->getchannel()->setholder(reqinfo);
        eloop->queueinloop(std::bind(&Httpdata::newevent, reqinfo));
    }
    acceptor->setevents(EPOLLIN | EPOLLET);
}