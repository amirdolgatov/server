#include <iostream>
#include "Server.h"


int main() {

    Server server{4046};

    server.createServer();
    server.start();

    return 0;
}
