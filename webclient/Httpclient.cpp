/* 简单的echo客户端程序 */

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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
}