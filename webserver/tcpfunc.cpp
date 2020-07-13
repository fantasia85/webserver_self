/* function used in the TCP connection */

#include "tcpfunc.h"
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

const int MAX_BUF = 4096;
const int LISTENQ = 2048;

/* read n bytes from a descriptor */
ssize_t readn(int fd, void *buf, size_t nbytes)
{
    size_t nleft = nbytes;
    ssize_t nread;
    char *ptr;

    ptr = static_cast<char *> (buf);
    while (nleft > 0) {
        if ((nread = read(fd, buf, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0; /* continue to read */
            else if (errno == EAGAIN) 
                return nbytes - nleft; /* resource temporarily unavailable. return amount read so far */
            else
                return -1; /* error. return -1 */
        }
        else if (nread == 0)
            break; /* EOF */

        nleft -= nread;
        ptr += nread;
    }
    return nbytes - nleft; 
}

ssize_t readn(int fd, std::string &sbuf, bool &iszero)
{
    ssize_t nread = 0;
    ssize_t readsum = 0;
    while (true) {
        char buf[MAX_BUF];
        if ((nread = read(fd, buf, MAX_BUF)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else if (errno == EAGAIN) 
                return readsum;
            else {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0) {
            iszero = true;
            break;
        }

        readsum += nread;
        sbuf += std::string(buf, buf + nread);
    }
    return readsum;
}

ssize_t readn(int fd, std::string &sbuf)
{
    ssize_t nread = 0;
    ssize_t readsum = 0;
    while (true) {
        char buf[MAX_BUF];
        if ((nread = read(fd, buf, MAX_BUF)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else if (errno == EAGAIN)
                return readsum;
            else {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0) 
            break;

        readsum += nread;
        sbuf += std::string(buf, buf + nread);
    }
    return readsum;
}

/* write n bytes to a descriptor */
ssize_t writen (int fd, void *buf, size_t nbytes)
{
    size_t nleft = nbytes;
    ssize_t nwritten;
    char *ptr = static_cast<char *> (buf);

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* continue */
            else if (nwritten < 0 && errno == EAGAIN)
                return nbytes - nleft;
            else 
                return -1;
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return nbytes - nleft;
}

ssize_t writen(int fd, std::string &sbuf)
{
    size_t nbytes = sbuf.size();
    ssize_t nwritten = 0;
    size_t nleft = nbytes;
    /* 使用了string中的c_str()函数，生成一个const char *指针，指向以空字符结束的数组 */ 
    const char *ptr = sbuf.c_str();
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else if (nwritten < 0 && errno == EAGAIN)
                break;
            else
                return -1;
        }

        nleft -= nwritten;
        ptr += nwritten;
    }

    /* 当所有字节全读取完成之后，清空sbuf。若未完全读取完成，则清除已经读取的字节 */
    if (nleft == 0)
        sbuf.clear();
    else
        sbuf = sbuf.substr(nbytes - nleft);
    
    return nbytes - nleft;
}

/*
void *bzero(void *ptr, size_t n)
{
    return memset(ptr, 0, n);
}
*/

/* set sigpipe signal as SIG_IGN */
void handle_sigpipe()
{
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL))
        return;
}

/* set socket nonblocking */
int setsocketnonblocking(int fd)
{
    int val;
    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        return -1;
    val |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, val) < 0)
        return -1;
    return 0;
}

/* 不使用Nagle算法 */
void setsocketnodelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

/* set socket nolinger. 当还有未发报文而套接字已关闭时，延迟时间 */
void setsocketnolinger(int fd)
{
    struct linger sktlinger;
    sktlinger.l_onoff = 1; /* on */
    sktlinger.l_linger = 30; /* linger time */
    setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&sktlinger, sizeof(sktlinger));
}

int tcp_listen(int port)
{
    if (port < 0 || port > 65535)
        return -1;

    int listenfd;
    struct sockaddr_in serveraddr;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* 消除address already in use的错误 */
    int val = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        close(listenfd);
        return -1;
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(static_cast<unsigned short>(port));

    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    
    if (listen(listenfd, LISTENQ) < 0)
        return -1;

    return listenfd;
}