#ifndef SKCHATROOM_CLIENT_HPP
#define SKCHARROOM_CLIENT_HPP

#include "common.hpp"
#include <string>

namespace skChatRoom{
    class Client{
    public:
        Client();
        connect_server();
        start();
        close_desc();

    private:
        //当前连接服务器端创建的套接字
        int sock;
        int pid;
        int epollfd;
        //管道pipe_fd[0]用于父进程读，pipe_fd[1]用于子进程写
        int pipe_fd[2];
        //客户端是否正常运行
        bool is_client_work;
        //聊天信息缓冲区
        char message[BUF_SIZE];
        //服务器的IP+port
        struct sockaddr_in server_addr;
    };
}


#endif
