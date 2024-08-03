#include <iostream>
#include <unistd.h>
#include "Server.h"
#include "Client.h"


int main() {

//    Server server{4046, 0.00001, 0.0, 10.0};
//
//    server.create_server();
//    server.start();
//
//    sleep(5);
//    server.stop();

    double left = 0.0;
    double right = 10.0;

//    std::vector<std::string> str;
//
//    splitMsg("0.32221 1.77765 100", str);
//
//
//    for (auto& it: str) {
//        std::cout << it << std::endl;
//    }

    Client client_("0.0.0.0", 7090);

    std::vector<Client> clients;
    std::vector<std::thread> th;

    Server server{7090, 0.00000001, 0.0, 10.0};
    server.create_server();
    server.start();

    for(int i = 0; i < 10; ++i){
        clients.emplace_back("0.0.0.0", 7090);
    }

    for (auto& client: clients) {
        client.create_client();
        std::thread thr{
            [&](){
                client.start();
            }
        };
        thr.detach();
        th.emplace_back(std::move(thr));
    }

    server.start_task();
    server.stop();



//    Iteration iteration{left, right, 2000};
//    iteration.client_number = 85;
//    iteration.task_create();
//
//    for (auto [id, task]: iteration.tasks)  {
//        std::cout << task.left << " " << task.right << " " << task.N << std::endl;
//    }

std::cout << std::endl;

    for (int i = 1; i < 200000; i = i * 2 ) {
//        std::cout << "N: " << i << ", s = " << (double)client.task_resolve(left, right, i) << std::endl;
        printf("N %d,\t s = %f\n", i, client_.task_resolve(left, right, i));
    }

    return 0;
}
