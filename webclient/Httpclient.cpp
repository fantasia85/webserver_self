/* 简单的echo客户端程序 */

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>

#define MAXSIZE 1024
#define IPADDRESS "127.0.0.1"
#define PORT 80
#define FDSIZE 1024

int setconnsocketnonblocking(int fd);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
    char buf[4096];
    buf[0] = '\0';

    const char *p = " ";
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == 0) {
        setconnsocketnonblocking(sockfd);
        std::cout << "1: " << std::endl;
        ssize_t n = write(sockfd, p, strlen(p));
        std::cout << "strlen(p) = " << strlen(p) << std::endl;
        sleep(1);
        n = read(sockfd, buf, 4096);
        std::cout << "n = " << n << std::endl;
        std::cout << buf;
        close(sockfd);
    }
    else
        perror("error1");
    sleep(1);

    p = "GET HTTP1.1";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
        setconnsocketnonblocking(sockfd);
        std::cout << "2: " << std::endl;
        ssize_t n = write(sockfd, p, strlen(p));
        std::cout << "strlen(p) = " << strlen(p) << std::endl;
        sleep(1);
        n = read(sockfd, buf, 4096);
        std::cout << "n = " << n << std::endl;
        std::cout << buf;
        close(sockfd); 
    }
    else
        perror("error2");
    sleep(1);

    p = "GET / HTTP/1.1\r\nHost: 127.0.0.1:80\r\nContent-Type: application/x-www-form-unlencoded\r\n"
        "Connection: Keep-Alive\r\n\r\n";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
        setconnsocketnonblocking(sockfd);
        std::cout << "3: " << std::endl;
        ssize_t n = write(sockfd, p, strlen(p));
        std::cout << "strlen(p) = " << strlen(p) << std::endl;
        sleep(1);
        n = read(sockfd, buf, 4096);
        std::cout << "n = " << n << std::endl;
        std::cout << buf;
        close(sockfd); 
    }
    else
        perror("error3");
    sleep(1);

    return 0;
}

int setconnsocketnonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
        return -1;
        
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
        return -1;

    return 0;
}