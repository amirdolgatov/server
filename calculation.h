//
// Created by amir on 23.07.24.
//

#ifndef MULTITHREADEDSERVER_CALCULATION_H
#define MULTITHREADEDSERVER_CALCULATION_H

#include "network_utils.h"
#include "queue_safe.h"
#include <future>
#include <unordered_map>

/* задание (подзадача) для клиента */
struct Task{
    int task_id{0};     //< идентификатор задачи (номер задачи)
    double left{0.0};   //< начало диапазона
    double right{0.0};  //< окончание диапазона
    int N{0};           //< количество точек на диапазоне
};

/* ответ от клиента */
struct Result{
    int task_id{0};         //< идентификатор задачи (номер задачи)
    bool success{false};    //< флаг успешного решения задачи
    float value{0.0};       //< результат вычисления
    int client_fd{0};       //< файловый сокет клиента-исполнителя
};


/*!
 * Класс Iteration выполнит одну итерацию
 * Итерация это расчет суммы с параметрами left, right, N
 * Задача (left, right, N) будет раздроблена на client_number подзадач,
 * client_number - количество клиентов, доступных в данный момент
 */
class Iteration{
public:
    /*!
     * Конструктор
     * @param left начало диапазона
     * @param right окончание диапазона
     * @param N количество точек на диапазоне
     */
    Iteration(double left, double right, int N):
    left{left}, right{right}, N{N}{}


    /*!
     * Метод в котором происходит взаимодействие с отдельным клиентом
     * @param client_fd сокет клиента
     * @param task задание для клиента
     * @return результат работы клиента
     */
    static Result client_interaction(int client_fd, Task task){
        auto& [id, left, right, N] = task;
        std::string result;                           /* сообщение с результатом расчета */
        /* сообщение для клиента: "[left] [right] [N]" */
        std::string task_str = std::to_string(left) + " " + std::to_string(right) + " " + std::to_string(N);
        if(!sendMsg(client_fd, task_str))             /* отправляем задание */
            return {id, false, 0.0, client_fd};
        if(!recvMsg(client_fd, result)){              /* ожидаем результат расчета */
            return {id, false, 0.0, client_fd};
        }
        return {id, true, std::stof(result), client_fd};
    }


    /*!
     * Метод асинхронного запуска решения всех подзадач
     * @param clients_queue очередь с клиентами, доступными в данный момент
     * @return результат расчета суммы (left, right, N)
     */
    double calculate_async(queue_safe<int>& clients_queue){

        this->client_number = clients_queue.size();   //< сколько исполнителей доступно в данный момент?

        task_create();                                //< создадим задания для каждого исполнителя

        while(!tasks.empty()){                        //< есть ли еще невыполненные задания ???
            for(auto& [id, task]: tasks){
                if(!clients_queue.empty()){
                    auto client_fd = *clients_queue.front_pop();        //< забираем сокет клиента из общей очереди
                    futures.emplace_back(std::async(std::launch::async, client_interaction, client_fd, task));
                }
            }
            for(auto& future: futures){
                auto [id, success, value, client_fd] = future.get();
                if(success){                                            //< если задача решена успешно, то
                    clients_queue.push(client_fd);                      //< возвращаем клиента в очередь
                    result += value;                                    //< увеличиваем сумму
                    tasks.erase(id);                                    //< отмечаем задачу выполненной, удаляем ее по идентификатору
                }
            }
        }
        return result;
    }


    /*!
     * Метод создания подзадач для каждого клиента
     *
     * Комментарий к алгоритму
     * Предполагается, что всегда N >= client_number. В таком случае имеем два возможных варианта:
     *
     *  1. N делится нацело на число client_number, в этом случае каждому клиенту достается равное число точек для расчета
     *  (то есть n = N / client_number точек каждому клиенту)
     *
     *  2. N делится на число client_number с остатком, тогда N % client_number != 0. В этом случае нескольким клиентам достанется
     *  n + 1 точек, остальным клиентам только n точек (а именно, первым N % client_number клиентам достанется n + 1 точка,
     *  а другим client_number - N % client_number достанется n точек)
     *
     */
    void task_create(){
        double l = this->left;   //< левый край диапазона
        double r = 0;            //< правый край диапазона
        int n = this->N / client_number;                            // количество точек для каждого клиента
        auto dx = (this->right - this->left) / N;                   // ширина дипазаона для одной точки

        int i = 0;                               // номер текущего элементарного участка
        for ( ; i < N % client_number; ++i) {    // раздали задания для первых N % client_number клиентов
            r = l + (n + 1) * dx;
            tasks.emplace(i, Task{
                    .task_id = i,
                    .left = l,
                    .right = r,
                    .N = n + 1
            });
            l = r;
        }

        for (  ; i < client_number; ++i) {     // раздали задания остальным client_number клиентам
            r = l + n * dx;
            tasks.emplace(i, Task{
                .task_id = i,
                .left = l,
                .right = r,
                .N = n
            });
            l = r;
        }
    }

    std::vector<std::future<Result>> futures;  // будущие результаты вычислений клиентов
    std::unordered_map<int, Task> tasks;       // подзадачи для клиентов

    double left{0.0};
    double right{0.0};
    int N{0};
    int client_number{0};

    double result{0.0};
};

#endif //MULTITHREADEDSERVER_CALCULATION_H
