#include "ProtocolHandler.h"
#include <sys/socket.h>
#include <stdexcept>

void ProtocolHandler::sendAuthResult(int socket, bool success) {
    const char* response = success ? "OK" : "ERR";
    send(socket, response, success ? 2 : 3, 0);
}

uint32_t ProtocolHandler::receiveUint32(int socket) {
    uint32_t value;
    if (!recv_all(socket, &value, sizeof(value))) {
        throw std::runtime_error("Failed to receive uint32");
    }
    return value;
}

uint64_t ProtocolHandler::receiveUint64(int socket) {
    uint64_t value;
    if (!recv_all(socket, &value, sizeof(value))) {
        throw std::runtime_error("Failed to receive uint64");
    }
    return value;
}

void ProtocolHandler::sendUint32(int socket, uint32_t value) {
    if (!send_all(socket, &value, sizeof(value))) {
        throw std::runtime_error("Failed to send uint32");
    }
}

void ProtocolHandler::sendUint64(int socket, uint64_t value) {
    if (!send_all(socket, &value, sizeof(value))) {
        throw std::runtime_error("Failed to send uint64");
    }
}

std::string ProtocolHandler::receiveAuthMessage(int socket) {
    char buffer[52];
    if (recv(socket, buffer, 52, 0) != 52) {
        throw std::runtime_error("Failed to receive auth message");
    }
    return std::string(buffer, 52);
}

bool ProtocolHandler::recv_all(int sock, void* buf, size_t len) {
    char* p = static_cast<char*>(buf);
    while(len > 0) {
        ssize_t received = recv(sock, p, len, 0);
        if(received <= 0) return false;
        p += received;
        len -= received;
    }
    return true;
}

bool ProtocolHandler::send_all(int sock, const void* buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    while(len > 0) {
        ssize_t sent = send(sock, p, len, 0);
        if(sent <= 0) return false;
        p += sent;
        len -= sent;
    }
    return true;
}
