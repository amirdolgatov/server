//
// Created by amir on 23.07.24.
//

#ifndef MULTITHREADEDSERVER_CALCULATION_H
#define MULTITHREADEDSERVER_CALCULATION_H

#include "network_utils.h"
#include "queue_safe.h"
#include <future>
#include <unordered_map>

struct Task{
    int id{0};
    float left{0.0};
    float right{0.0};
    int N{0};
};

struct Result{
    float value{0.0};
    bool success{false};
    int executor{0};
};

class Iteration{



    static Result client_interaction(int client_fd, Task task){
        auto& [id, left, right, N] = task;
        std::string result;
        std::string task_str = std::to_string(left) + std::to_string(right) + std::to_string(N);
        if(!sendMsg(client_fd, task_str))
            return {0.0, false, client_fd};
        if(!recvMsg(client_fd, result)){
            // delete_client_fd();
            return {0.0, false, client_fd};
        }

        return {std::stof(result), true, client_fd};
    }

    void calculation(queue_safe<int>& clients_queue){
        for(auto& [id, task]: tasks){
            if(!clients_queue.empty()){
                auto client_fd = *clients_queue.front_pop();
                futures.emplace_back(std::async(client_interaction, client_fd, task));
            }
        }

        for(auto& future: futures){
            auto [value, success, client_fd]  = future.get();
            if(success){

            }
        }
            ;
    }

    std::vector<std::future<Result>> futures;
    std::unordered_map<int, Task> tasks;

};

#endif //MULTITHREADEDSERVER_CALCULATION_H
