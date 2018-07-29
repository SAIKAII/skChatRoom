#ifndef SKCHATROOM_SERVER_HPP
#define SKCHATROOM_SERVER_HPP

#include "common.hpp"

namespace skChatRoom{
    class Server{
    public:
        Server();
        void init();
        void start();
        void close_desc();

    private:
        //广播信息
        int send_broadcast_message(int clientfd);
        //服务端的地址信息
        struct sockaddr_in server_address;
        //监听的描述符
        int listenfd;
        //内核事件表描述符
        int epollfd;
        //客户端列表
        list<int> clients_list;
    };
}


#endif
