#include "Server.h"
#include <unistd.h>
#include <arpa/inet.h>

Server::Server(const Config& config) 
    : port(config.getPort()), 
      userDb(config.getUserDbFile()), 
      logger(config.getLogFile()),
      running(false),
      serverSocket(-1) {}

void Server::start() {
    if (!userDb.load()) {
        logger.log(LogLevel::ERROR, "Не удалось загрузить базу пользователей");
        return;
    }
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        logger.log(LogLevel::ERROR, "Ошибка создания сокета");
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        logger.log(LogLevel::ERROR, "Ошибка привязки сокета к порту " + std::to_string(port));
        close(serverSocket);
        return;
    }
    
    if (listen(serverSocket, 5) < 0) {
        logger.log(LogLevel::ERROR, "Ошибка начала прослушивания");
        close(serverSocket);
        return;
    }
    
    running = true;
    logger.log(LogLevel::INFO, "Сервер запущен на порту " + std::to_string(port));
    
    acceptConnections();
}

void Server::stop() {
    running = false;
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    logger.log(LogLevel::INFO, "Сервер остановлен");
}

void Server::acceptConnections() {
    while (running) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) {
            if (running) {
                logger.log(LogLevel::WARNING, "Ошибка принятия соединения");
            }
            continue;
        }
        
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        logger.log(LogLevel::INFO, "Новое подключение от " + std::string(clientIp));
        
        ClientSession session(clientSocket, userDb, logger);
        session.run();
    }
}
