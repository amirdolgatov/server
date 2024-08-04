#include <iostream>
#include <unistd.h>
#include "Server.h"
#include "Client.h"

/*!
 * Пример вычислений
 */
int main() {

    int port = 7090;

    double left = 0.0;
    double right = 1.0;
    double precision = 0.000000000000000000000000000000000001;


    int cli_number = 50;
    std::vector<Client> clients;
    std::vector<std::thread> client_threads;

    /*!
     * Создаем, запускаем сервер
     */
    Server server{port, precision, left, right};
    server.create_server();
    server.start();

    /*!
     * Создаем клиентов
     */
    for(int i = 0; i < cli_number; ++i){
        clients.emplace_back("0.0.0.0", port);
    }

    /*!
     * Запускаем клиентов в отдельных потоках
     */
    for (auto& client: clients) {
        client.create_client();
        std::thread thr{
            [&](){
                client.start();
            }
        };
        thr.detach();
        client_threads.emplace_back(std::move(thr));
    }

    // запуск решения задачи
    server.start_task();

    server.stop();

    return 0;
}
