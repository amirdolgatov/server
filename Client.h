//
// Created by amir on 26.07.24.
//

#ifndef MULTITHREADEDSERVER_CLIENT_H
#define MULTITHREADEDSERVER_CLIENT_H

#include "network_utils.h"
#include <vector>
#include <cmath>

/*!
 * Описание клиента - исполнителя
 */
class Client{
public:

    /*!
     * Конструктор
     * @param addr адрес сервера
     * @param port порт сервера
     */
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

    /*!
     * Фактическое создание клиента, вынесено в отдельный метод
     */
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


    /*!
     * Запуск взаимодействия с сервером
     */
    void start(){
        std::string input;                          //< входящее сообщение
        std::string output;                         //< исхоядщее сообщение
        while(recvMsg(sockfd, input)){              //< получено задание
            auto result = command_handler(input);   //< распарсить задание, выполнить его
            output = std::to_string(result);
            sendMsg(sockfd, output);                //< вернуть ответ серверу
            input.clear();
            output.clear();
        }
    }



    /*!
     * Обработка сообщения (команды) и решение задачи
     * @param msg сообщение сервера
     * @return значение интеграла
     */
    double command_handler(const std::string& msg){
        std::vector<std::string> parameters;
        splitMsg(msg, parameters);
        auto left = std::stod(parameters[0]);
        auto right = std::stod(parameters[1]);
        auto N = std::stod(parameters[2]);
        if(left < right && N > 0){
            return task_resolve(left, right, N);
        }
        else
            return 0.0;
    }


    /*!
     * Вычисление интеграла
     * @param left нижний предел интегрирования
     * @param right верхний предел интегрирования
     * @param N количество точек
     * @return сумма
     */
    double task_resolve(double left, double right, int N){
        double l = left;
        double summ = 0.0;
        double dx = (right - left) / N;
        for (int i = 0; i < N; ++i) {
            summ += f(l + (i + 0.5) * dx);
        }
        return summ * dx;
    }

    /*!
     * Подинтегральная функция
     * @param arg аргумент функции
     * @return
     */
    double f(double arg){
        double epsilon = 0.000000000000000000000001;
        return sin( 1./ (arg + epsilon));
//        return sin(arg);
    }

    int sockfd;
    struct sockaddr_in servaddr;

};

#endif //MULTITHREADEDSERVER_CLIENT_H
