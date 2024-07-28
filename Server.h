//
// Created by amir on 17.07.24.
//

#ifndef MULTITHREADEDSERVER_SERVER_H
#define MULTITHREADEDSERVER_SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>

#include "queue_safe.h"
#include "calculation.h"


class Server{
public:

    Server(int port, double precision, double left, double right):
    port{port}, precision{precision}, left{left}, right{right}
    {
        /* Считаем, что пределы интегрирования должны удовлетворять данным условиям */
        if(left >= right || right - left <= precision ){
            data_is_correct = false;
            std::cout << "Warning: incorrect interval [left, right]" << std::endl;
        }
        /* Если precision < 0, то будет осуществлена только одна итерация */
        if(precision < 0){
            data_is_correct = false;
            std::cout << "Warning: precision must be positive" << std::endl;
        }

        data_is_correct = true;
    }

    void create_server(){

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;  // использовать IPv4
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

    // Пуск сервера
    void start(){

        is_up = true;

        std::thread new_client_handler{
            [&](){
                int client_sockfd;
                struct sockaddr address;
                auto addrlen = sizeof(sockaddr_in);

                while(is_up){
                    client_sockfd = accept(sockfd, &address, (socklen_t *)&addrlen);

                    if(set_keep_alive(client_sockfd))
                        client_sockets.push(client_sockfd);
                }
            }
        };
        new_client_handler.detach();
    }

    void stop(){
        is_up = false;
    }

    // Пуск решения задачи
    double start_task(){
        if(data_is_correct)
            std::cout << "RESULT: " << сalculate_task() << std::endl;
        else{
            std::cout << "Change input data" << std::endl;
        }
    }

    // Метод асинхронного, многопоточного вычисления
    double сalculate_task(){

        double current_summ{0.0};
        double previous_summ{0.0};

        for( ; this->N <= client_sockets.size(); this->N = this->N * 2)
            ;

        do {
            previous_summ = current_summ;
            Iteration iteration{this->left, this->right, this->N};
            current_summ = iteration.calculate_async(client_sockets);
        }
        while(current_summ - previous_summ > this->precision);

        close_connections();

        return current_summ;
    }

    void close_connections(){
        while(!this->client_sockets.empty()){
            auto sock = client_sockets.front_pop();
            close(*sock);
        }
    }


    bool set_keep_alive(int fd)
    {
        int val = 1;

        // Turn on the keepalive mechanism
        if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
        {
            std::cout << strerror(errno);
            return false;
        }

        /* Send first probe after interval. */
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
            std::cout << strerror(errno);
            return false;
        }

        /* Send next probes after the specified interval. */
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
            std::cout << strerror(errno);
            return false;
        }

        /* Consider the socket in error state after three we send three ACK
         * probes without getting a reply. */
        val = 2;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
            std::cout << strerror(errno);
            return false;
        }

        /* Success */
        return true;
    }





private:

    bool is_up{false};
    struct addrinfo hints, *res;
    int sockfd;// сначала заполним адресные структуры ф-ей getaddrinfo():
    int port;

    queue_safe<int> client_sockets;

    double left{0.0};
    double right{0.0};
    double precision{0.0};
    int N{1024};

    bool data_is_correct;


};

#endif //MULTITHREADEDSERVER_SERVER_H
