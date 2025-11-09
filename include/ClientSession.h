#pragma once

#include "UserDatabase.h"
#include "Logger.h"
#include "ProtocolHandler.h"
#include "CryptoService.h"
#include <vector>
#include <cstdint>

class ClientSession {
private:
    int clientSocket;
    UserDatabase& userDb;
    Logger& logger;
    bool authenticated;

    bool authenticate();
    void processData();
    std::vector<uint64_t> receiveVector();
    uint64_t calculateSum(const std::vector<uint64_t>& vec);
    std::string toUpper(const std::string& str);

public:
    ClientSession(int socket, UserDatabase& database, Logger& log);
    void run();
    bool isAuthenticated() const;
};
