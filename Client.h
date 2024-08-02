//
// Created by amir on 26.07.24.
//

#ifndef MULTITHREADEDSERVER_CLIENT_H
#define MULTITHREADEDSERVER_CLIENT_H

#include "network_utils.h"
#include <vector>
#include <cmath>

class Client{
public:

    Client(const std::string addr, int port){
        memset(&servaddr, 0, sizeof(servaddr));
        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(addr.c_str());
        servaddr.sin_port = htons(port);
    }

    ~Client(){
        close(sockfd);
    }

    void create_client(){
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            perror("socket:");
            exit(1);
        }

        if(connect(sockfd,(struct sockaddr *)&servaddr, \
					sizeof(struct sockaddr)) == -1){
            perror("connect:");
            exit(1);
        }
    }

    void start(){
        std::string input;
        std::string output;
        while(recvMsg(sockfd, input)){
            std::cout << "I'am " << sockfd << ", I get task" << std::endl;
            auto result = command_handler(input);
            output = std::to_string(result);
            std::cout << "answer " << output << std::endl;
            sendMsg(sockfd, output);
            std::cout << "I send answer" << std::endl;
            input.clear();
            output.clear();
        }
    }

    double command_handler(const std::string& msg){
        std::cout << "I'm going to resolve task " << std::endl;
        std::vector<std::string> commands;
        splitMsg(msg, commands);
        auto left = std::stod(commands[0]);
        auto right = std::stod(commands[1]);
        auto N = std::stod(commands[2]);
        std::cout << "I'm going to resolve task " << std::endl;
        if(left < right && N > 0){
            return task_resolve(left, right, N);
        }
        else
            return 0.0;
    }

    double task_resolve(double left, double right, int N){
        double l = 0.0;
        double summ = 0.0;
        double dx = (right - left) / N;
        for (int i = 0; i < N; ++i) {
            summ += f(l + (i + 0.5) * dx);
        }
        return summ * dx;
    }

    double f(double x){
        return x*x - x;
    }

    int sockfd;
    struct sockaddr_in servaddr;

};

#endif //MULTITHREADEDSERVER_CLIENT_H
