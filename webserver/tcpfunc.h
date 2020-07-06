/* function used in the TCP connection */

#pragma once

#include <cstdlib>
#include <string>

/* readn和writen分别读，写指定的nbytes字节数据，并处理返回值可能小于要求值的情况 */
ssize_t readn(int fd, void *buf, size_t nbytes);
ssize_t readn(int fd, std::string &sbuf, bool &iszero);
ssize_t readn(int fd, std::string &sbuf);

ssize_t writen(int fd, void *buf, size_t nbytes);
ssize_t writen(int fd, std::string &sbuf);

/*void *bzero(void *ptr, size_t n); */

void handle_sigpipe();

int setsocketnonblocking(int fd);
void setsocketnodelay(int fd);
void setsocketnolinger(int fd);

/* tcp_listen执行TCP服务器的通常步骤：创建一个TCP套接字，给它捆绑服务器的
 * 众所周知的端口，并允许接受外来的连接请求。 */
int tcp_listen(int port);