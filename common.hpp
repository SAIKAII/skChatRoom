#ifndef SKCHATROOM_COMMON_HPP
#define SKCHATROOM_COMMON_HPP

#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>
#include <arpa/inet.h>
#include <list>
#include <error.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "12345"
#define EPOLL_SIZE 5000
#define SERVER_WELCOME "Welcome to join this chat room! Your chat ID is: #%d"
#define SERVER_MESSAGE "ClientID %d say >> %s"
#define BUF_SIZE 0xFFFF
#define CAUTION "There is only you in this room!"

//设置套接字为非阻塞模式
int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将套接字添加到内核事件表中
void addfd(int epollfd, int fd, bool enable_et){
    struct epoll_event event;

    event.data.fd = fd;
    event.events = EPOLLIN;
    if(true == enable_et){
        event.events |= EPOLLET;
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);   
}


#endif
