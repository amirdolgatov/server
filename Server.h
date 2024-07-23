//
// Created by amir on 17.07.24.
//

#ifndef MULTITHREADEDSERVER_SERVER_H
#define MULTITHREADEDSERVER_SERVER_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>


class Server{
public:

    Server(int port):port{port}{}

    void createServer(){

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;  // использовать IPv4 или IPv6, нам неважно
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;     // Заполните для меня мой IP
        getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &res);
        // создаем сокет:
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        int yes = 1;
        // избавляемся от надоедливой ошибки «Address already in use»
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // вешаем его на порт, который мы передали в getaddrinfo():
        if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
            std::cout << "Can't bind socket" << std::endl;

        if(listen(sockfd, 10000) == -1){
            std::cout << "Server: Can't listen port" << std::endl;
        }
    }

    void start(){

        is_up = true;

        int client_sockfd;
        struct sockaddr address;
        auto addrlen = sizeof(sockaddr_in);

        while(is_up){

            client_sockfd = accept(sockfd, &address, (socklen_t *)&addrlen);

            client_sockets.push_back(client_sockfd);

            std::thread thr{client_handler, client_sockfd, std::ref(is_up)};
            thr.detach();

            connections.emplace_back(std::move(thr));

        }

    }

    static void client_handler(int sockfd, bool& serv_is_up){
        bool stop = false;
        char buffer[150];
        int buffer_ptr = 0;

        std::string msg = "Hello! Send me something\n\r";

        sendMsg(sockfd, msg);

        while(!stop && serv_is_up){
            if(recv_str(sockfd, buffer, 150, buffer_ptr)){
                if(send_str(sockfd, buffer, buffer_ptr))
                    continue;
                }
            else
                break;
        }
    }






private:

    bool is_up{false};
    struct addrinfo hints, *res;
    int sockfd;// сначала заполним адресные структуры ф-ей getaddrinfo():
    int port;
    std::vector<int> client_sockets;
    std::vector<std::thread> connections;

};

#endif //MULTITHREADEDSERVER_SERVER_H
