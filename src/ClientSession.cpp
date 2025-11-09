#include "ClientSession.h"
#include <unistd.h>
#include <algorithm>
#include <stdexcept>

ClientSession::ClientSession(int socket, UserDatabase& database, Logger& log) 
    : clientSocket(socket), userDb(database), logger(log), authenticated(false) {}

void ClientSession::run() {
    try {
        if (authenticate()) {
            processData();
        }
    } catch (const std::exception& e) {
        logger.log(LogLevel::ERROR, "Ошибка в сессии: " + std::string(e.what()));
    }
    
    close(clientSocket);
}

bool ClientSession::isAuthenticated() const {
    return authenticated;
}

bool ClientSession::authenticate() {
    std::string authMessage = ProtocolHandler::receiveAuthMessage(clientSocket);
    
    if (authMessage.length() != 52) {
        logger.log(LogLevel::WARNING, "Неверный формат auth сообщения");
        ProtocolHandler::sendAuthResult(clientSocket, false);
        return false;
    }
    
    std::string login = authMessage.substr(0, 4);
    std::string salt = authMessage.substr(4, 16);
    std::string clientHash = authMessage.substr(20, 32);
    
    logger.log(LogLevel::INFO, "Аутентификация для пользователя: " + login);
    
    if (!userDb.isValidUser(login)) {
        logger.log(LogLevel::WARNING, "Пользователь не найден: " + login);
        ProtocolHandler::sendAuthResult(clientSocket, false);
        return false;
    }
    
    std::string password = userDb.getPassword(login);
    std::string computedHash = CryptoService::computeMD5(salt + password);
    
    std::string clientHashUpper = toUpper(clientHash);
    std::string computedHashUpper = toUpper(computedHash);
    
    if (clientHashUpper != computedHashUpper) {
        logger.log(LogLevel::WARNING, "Неверный пароль для пользователя: " + login);
        ProtocolHandler::sendAuthResult(clientSocket, false);
        return false;
    }
    
    authenticated = true;
    ProtocolHandler::sendAuthResult(clientSocket, true);
    logger.log(LogLevel::INFO, "Успешная аутентификация: " + login);
    return true;
}

void ClientSession::processData() {
    uint32_t numVectors = ProtocolHandler::receiveUint32(clientSocket);
    logger.log(LogLevel::INFO, "Количество векторов: " + std::to_string(numVectors));
    
    for (uint32_t i = 0; i < numVectors; i++) {
        std::vector<uint64_t> vector = receiveVector();
        uint64_t sum = calculateSum(vector);
        ProtocolHandler::sendUint64(clientSocket, sum);
        logger.log(LogLevel::INFO, "Вектор " + std::to_string(i + 1) + " обработан, сумма: " + std::to_string(sum));
    }
}

std::vector<uint64_t> ClientSession::receiveVector() {
    uint32_t size = ProtocolHandler::receiveUint32(clientSocket);
    std::vector<uint64_t> vector(size);
    
    for (uint32_t i = 0; i < size; i++) {
        vector[i] = ProtocolHandler::receiveUint64(clientSocket);
    }
    
    return vector;
}

uint64_t ClientSession::calculateSum(const std::vector<uint64_t>& vec) {
    uint64_t sum = 0;
    for (auto val : vec) {
        if (val > UINT64_MAX - sum) {
            return UINT64_MAX;
        }
        sum += val;
    }
    return sum;
}

std::string ClientSession::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}
