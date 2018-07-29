#include <iostream>
#include "client.hpp"

using namespace skChatRoom;

Client::Client(){
    //初始化要连接的服务器的地址和端口
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    sock = 0;
    pid = 0;
    is_client_work = true; 
    epollfd = 0;
}

void Client::connect_server(){
    std::cout << "Connect server:" << SERVER_IP << ":" << SERVER_PORT << std::endl;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("socket failure");
        exit(-1);
    }

    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect failure");
        exit(-1);
    }

    //创建管道，其中[0]用于父进程读，[1]用于子进程写
    if(pipe(&pipe_fd) < 0){
        perror("pipe failure");
        exit(-1);
    }

    epollfd = epoll_create(EPOLL_SIZE);
    if(epollfd < 0){
        perror("epoll_create failure");
        exit(-1);
    }

    addfd(epollfd, sock, true);
    addfd(epollfd, pipe_fd[0], true);
}

void Client::close_desc(){
    if(0 == pid){
        close(pipe_fd[1]);
    }else{
        close(pipe_fd[0]);
        close(sock);
    }
}

void Client::start(){
    struct epoll_event events[2];

    //连接服务器
    connect_server();

    pid = fork();

    if(pid < 0){
        perror("fork failure");
        close(sock);
        exit(-1);
    }else if(0 == pid){
        //关闭子进程不使用的管道
        close(pipe_fd[0]);
        std::cout << "Please input 'exit' to exit the chat room" << std::endl;

        while(is_client_work){
            bzero(message, BUF_SIZE);
            fgets(message, BUF_SIZE, stdin);

            if(0 == strncasecmp(message, EXIT, strlen(EXIT))){
                is_client_work = false;
            }else{
                if(write(pipe_fd[1], message, strlen(message)) < 0){
                    perror("write failure");
                    exit(-1);
                }
            }
        }
    }else{
        close(pipe_fd[1]);

        while(is_client_work){
            int epoll_events_count = epoll_wait(epollfd, events, 2, -1);

            for(int i=0; i < epoll_events_count; i++){
                bzero(message, BUF_SIZE);
                
                if(events[i].data.fd == sock){
                    int ret = recv(sock, message, BUF_SIZE, 0);
                    if(0 == ret){
                        std::cout << "Server close connect: " << sock << std::endl;
                        close(sock);
                        is_client_work = false;
                    }else{
                        std::cout << message << std::endl;
                    }
                }else{
                    int ret = read(events[i].data.fd, message, BUF_SIZE);
                    
                    if(0 == ret){
                        is_client_work = false;
                    }else{
                        send(sock, message, BUF_SIZE, 0);
                    }
                }
            }
        }
    }

    close_desc();
}
