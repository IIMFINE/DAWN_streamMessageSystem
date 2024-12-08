#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int sock_fd;
    struct sockaddr_in server_addr;
    char buf[BUFSIZE];
    int bytes_sent, bytes_received;

    // 创建 socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // bind any port
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(0);
    if (bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(25000);

    // set sock_fd no block
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    // set keep alive
    int keep_alive = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));

    // set TCP_KEEPIDLE
    int keep_idle = 1;
    setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle));

    // 连接服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        // perror("connect");
        // exit(EXIT_FAILURE);
        if (errno != EINPROGRESS)
        {
            perror("connect");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("errno: %s", strerror(errno));
        }
    }

    printf("Connected to server.\n");

    // add sock_fd to epoll
    // 创建 epoll 实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    // epoll wait
    struct epoll_event events[10];
    while (1)
    {
        int nfds = epoll_wait(epoll_fd, events, 10, 500);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].events & EPOLLIN)
            {
                // printf("EPOLLIN\n");
            }
            if (events[i].events & EPOLLOUT)
            {
                // printf("EPOLLOUT\n");
            }
        }
    }

    while (1)
    {
        sleep(1);
    }
}
