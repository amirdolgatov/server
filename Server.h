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

#define MSG_SIZE 40

struct Task{
    float left{0.0};
    float right{0.0};
    int N{0};
};

using Result = std::pair<float, bool>;

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


    bool static sendMsg(int sockfd, std::string& str){
        int bytes = 0;
        int msg_len = str.size();
        const char *msg = str.c_str();
        while(msg_len > 0){

            if((bytes = send(sockfd, &msg[bytes], msg_len, 0)) != -1)
                msg_len -= bytes;
            else{
                std::cout << "Server: Can't send message to client" << std::endl;
                return false;
            }
        }
        return true;
    }

    bool static send_str(int sockfd, char* buffer, int msg_len){
        size_t bytes = 0;
        while(msg_len > 0){

            if((bytes = send(sockfd, buffer + bytes, msg_len, 0)) != -1)
                msg_len -= bytes;
            else{
                std::cout << "Server: Can't send message to client" << std::endl;
                return false;
            }
        }
        return true;
    }

    bool static recv_str(int sockfd, char* buffer, int len, int& received_bytes){
        if((received_bytes = recv(sockfd, buffer, len, 0)) <= 0)
            return false;
        return true;
    }

    static bool recvMsg(int sockfd, std::string& str){
        auto received_bytes = 0;
        char buffer[MSG_SIZE];
        if(!recv_str(sockfd, buffer, MSG_SIZE, received_bytes))
            return false;
        for (int i = 0; i < received_bytes; ++i) {
            str.push_back(buffer[i]);
        }
        return true;
    }

    static Result client_interaction(int client_fd, Task task){
        auto& [left, right, N] = task;
        std::string result;
        std::string task_str = std::to_string(left) + std::to_string(right) + std::to_string(N);
        if(!sendMsg(client_fd, task_str))
            return {0.0, false};
        if(!recvMsg(client_fd, result)){
            // delete_client_fd();
            return {0.0, false};
        }

        return {std::stof(result), true};
    }

    void start_calculation(){
        for(auto fd: client_sockets){
            futures.emplace_back(std::async(client_interaction, fd, Task{}))  ;
        }

    }

private:

    bool is_up{false};
    struct addrinfo hints, *res;
    int sockfd;// сначала заполним адресные структуры ф-ей getaddrinfo():
    int port;
    std::vector<int> client_sockets;
    std::vector<std::thread> connections;
    std::vector<std::future<Result>> futures

};

#endif //MULTITHREADEDSERVER_SERVER_H
