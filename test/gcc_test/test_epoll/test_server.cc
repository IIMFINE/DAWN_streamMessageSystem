#include <arpa/inet.h>
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

#include <iostream>

#define MAX_EVENTS 1024
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buf[BUFSIZE];
    int epoll_fd, nfds, i;
    struct epoll_event event, events[MAX_EVENTS];

    // 创建 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(25000);

    // set server_fd no block
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // set reuse
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // set keep alive
    int keep_alive = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));

    // set TCP_KEEPIDLE
    int keep_idle = 1;
    setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle));

    // 绑定 socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听 socket
    if (listen(server_fd, 10) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // 创建 epoll 实例
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // 将 server_fd 添加到 epoll 监听
    event.data.fd = server_fd;
    event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // 等待事件发生
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        std::cout << "epoll wait" << std::endl;

        // 处理事件
        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                std::cout << "accept" << std::endl;
                // 有新连接到来
                client_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1)
                {
                    perror("accept");
                    continue;
                }

                // 将新的客户端 socket 添加到 epoll 监听
                event.data.fd = client_fd;
                event.events = EPOLLIN;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    perror("epoll_ctl");
                    close(client_fd);
                    continue;
                }

                printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }
            else
            {
                // 有数据到达
                memset(buf, 0, BUFSIZE);
                int bytes_read = read(events[i].data.fd, buf, BUFSIZE);
                if (bytes_read == -1)
                {
                    perror("read");
                    close(events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    continue;
                }
                else if (bytes_read == 0)
                {
                    // 客户端连接已关闭
                    printf("Client disconnected\n");
                    close(events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    continue;
                }
                else
                {
                    // 处理读取到的数据
                    printf("Received: %s\n", buf);
                    // 这里可以添加您的业务逻辑
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
