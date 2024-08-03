//
// Created by amir on 23.07.24.
//

#ifndef MULTITHREADEDSERVER_CALCULATION_H
#define MULTITHREADEDSERVER_CALCULATION_H

#include "network_utils.h"
#include "queue_safe.h"
#include <future>
#include <unordered_map>

/* задание клиенту */
struct Task{
    int task_id{0};
    double left{0.0};
    double right{0.0};
    int N{0};
};

/* ответ от клиента */
struct Result{
    int task_id{0};
    bool success{false};
    float value{0.0};
    int client_fd{0};
};


/* Класс выполнит одну итерацию
 * Итерация это расчет суммы с параметрами left, right, N
 * N точек будет распределено на количество клиентов
 */
class Iteration{
public:

    Iteration(double left, double right, int N):
    left{left}, right{right}, N{N}{}

    // метод в котором происходит взаимодействие с клиентом
    static Result client_interaction(int client_fd, Task task){
        auto& [id, left, right, N] = task;
        std::string result;                           /* сообщение с результатом расчета */
        std::string task_str = std::to_string(left) + " " + std::to_string(right) + " " + std::to_string(N); /* сообщение "left right N" */
        if(!sendMsg(client_fd, task_str))
            return {id, false, 0.0, client_fd};

        if(!recvMsg(client_fd, result)){             /* сообщение с результатом расчета */
            return {id, false, 0.0, client_fd};
        }

        return {id, true, std::stof(result), client_fd};
    }

    // асинхронное выполнение задач
    double calculate_async(queue_safe<int>& clients_queue){

        this->client_number = clients_queue.size();

        task_create();

        while(!tasks.empty()){

            for(auto& [id, task]: tasks){
                if(!clients_queue.empty()){
                    auto client_fd = *clients_queue.front_pop();
                    futures.emplace_back(std::async(std::launch::async, client_interaction, client_fd, task));
                }
            }

            for(auto& future: futures){
                auto [id, success, value, client_fd] = future.get();
                if(success){
                    clients_queue.push(client_fd);
                    result += value;
                    tasks.erase(id);
                }
            }
        }

        return result;
    }

    // создаем задачи для каждого клиента
    void task_create(){
        double l = this->left;
        double r = 0;
        int n = this->N / client_number;                            // количество точек для каждого клиента
        auto dx = (this->right - this->left) / N;                   // ширина дипазаона для одной точки
        int k = 0;

        int i = 0;
        for ( ; i < N % client_number; ++i) {    // раздали задания
            r = l + (n + 1) * dx ;
            tasks.emplace(i, Task{
                    .task_id = i,
                    .left = l,
                    .right = r,
                    .N = n + 1
            });
            k = k + n + 1;
            std::cout << "Task create " << i << " " << l << " " << r << " " << n + 1 <<  " " << k << std::endl;
            l = r;
        }

        for (  ; i < client_number; ++i) {
            r = l + n * dx;
            tasks.emplace(i, Task{
                .task_id = i,
                .left = l,
                .right = r,
                .N = n
            });
            k = k + n;
            std::cout << "Task create " << i << " " << l << " " << r << " " << n << " " << k << std::endl;
            l = r;
        }
    }

    std::vector<std::future<Result>> futures;
    std::unordered_map<int, Task> tasks;

    double left{0.0};
    double right{0.0};
    int N{0};
    int client_number{0};

    double result{0.0};
};

#endif //MULTITHREADEDSERVER_CALCULATION_H
