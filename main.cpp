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
    double right = 6.0;
    Client client{"1.1.1.1", 7090};
    for (int i = 1; i < 200000; i = i * 2 ) {
//        std::cout << "N: " << i << ", s = " << (double)client.task_resolve(left, right, i) << std::endl;
        printf("N %d,\t s = %f\n", i, client.task_resolve(left, right, i));
    }

    return 0;
}
