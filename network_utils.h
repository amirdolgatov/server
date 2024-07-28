//
// Created by amir on 23.07.24.
//

#ifndef MULTITHREADEDSERVER_NETWORK_UTILS_H
#define MULTITHREADEDSERVER_NETWORK_UTILS_H

#include<iostream>


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define MSG_SIZE 40

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

static bool splitMsg(const std::string& msg, std::vector<std::string>& tmp_vec){
    std::string tmp_str;
    int i = 0;
    while(i != msg.size()){
        for ( ; msg[i] != ' ' && std::isdigit(msg[i]); tmp_str.push_back(msg[i++]))
            ;
        tmp_vec.push_back(tmp_str);
        tmp_str.clear();
    }
}

#endif //MULTITHREADEDSERVER_NETWORK_UTILS_H
