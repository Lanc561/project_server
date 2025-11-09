#pragma once

#include "Config.h"
#include "UserDatabase.h"
#include "Logger.h"
#include "ClientSession.h"
#include <sys/socket.h>
#include <netinet/in.h>

class Server {
private:
    int port;
    UserDatabase userDb;
    Logger logger;
    bool running;
    int serverSocket;

    void acceptConnections();

public:
    Server(const Config& config);
    void start();
    void stop();
};
