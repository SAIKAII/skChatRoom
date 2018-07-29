#include <iostream>
#include "server.hpp"

using namespace skChatRoom;

Server::Server(){
    //初始化服务器地址和端口
    server_address.sin_family = PF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    listenfd = 0;
    epollfd = 0;
}

void Server::init(){
    std::cout << "Init server ... \n";

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        perror("listenfd");
        exit(-1);
    }

    if(bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
        perror("bind error");
        exit(-1);
    }

    int ret = listen(listenfd, 5);
    if(ret < 0){
        perror("listen error");
        exit(-1);
    }

    std::cout << "Start to listen ... " << SERVER_IP << std::endl;

    epollfd = epoll_create(EPOLL_SIZE);
    if(epollfd < 0){
        perror("epollfd error");
        exit(-1);
    }

    addfd(epollfd, listenfd, true);

}

void Server::close_desc(){
    close(listenfd);
    close(epollfd);
}

int Server::send_broadcast_message(int clientfd){
    //buf用来接受信息，message用来保存格式化后的信息
    char buf[BUF_SIZE], message[BUF_SIZE];

    std::cout << "read from client(clientID = " << clientfd << ")\n";

    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);
    int len = recv(clientfd, buf, BUF_SIZE-1, 0);

    if(0 == len){
        close(clientfd);
        clients_list.remove(clientfd);
        std::cout << "clientID = " << clientfd << " closed\nnow there are"
            << clients_list.size() << " clients in this chat room.\n";
    }else{
        if(1 == clients_list.size()){
            send(clientfd, CAUTION, sizeof(CAUTION), 0);
            return len;
        }

        sprintf(message, SERVER_MESSAGE, clientfd, buf);
        for(auto client = clients_list.begin(); client != client_list.end(); client++){
            if(*client != clientfd){
                if(send(*client, message, BUF_SIZE, 0) < 0){
                    return -1;
                }
            }
        }
    }
    return len;
}

void Server::start(){
    static struct epoll_event events[EPOLL_SIZE];

    Init();

    while(true){
        int epoll_events_count = epoll_wait(epollfd, events, EPOLL_SIZE, -1);

        if(0 > epoll_events_count){
            perror("epoll error");
            break;
        }

        std::cout << "epoll_events_count = \n" << epoll_events_count << std::endl;

        for(int i=0; i < epoll_events_count; i++){
            int sockfd = events[i].data.fd;

            //接受一个连接
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addr_length = sizeof(struct sockaddr_in);
                int clientfd = accept(listenfd, &client_address, &client_addr_length);

                std::cout << "client connection from : "
                    << inet_ntoa(client_address.sin_addr) << ", clientfd = "
                    << clientfd << endl;

                addfd(clientfd);

                clients_list.push_back(clientfd);
                std::cout << “Add new clientfd = " << clientfd << " to epoll" << std::endl;
                std::cout << "Now there are " << clients_list.size() << " clients in this chat room.\n";

                std::cout << "Welcome message\n";
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(0 > ret){
                    perror("send failure");
                    close();
                    exit(-1);
                }
            }else{
                int ret = send_broadcast_message(clientfd);
                if(0 > ret){
                    perror("error");
                    close();
                    exit(-1);
                }
            }
        }
    }
    close();
}
