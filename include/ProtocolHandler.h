#pragma once

#include <string>
#include <cstdint>

class ProtocolHandler {
public:
    static void sendAuthResult(int socket, bool success);
    static uint32_t receiveUint32(int socket);
    static uint64_t receiveUint64(int socket);
    static void sendUint32(int socket, uint32_t value);
    static void sendUint64(int socket, uint64_t value);
    static std::string receiveAuthMessage(int socket);

private:
    static bool recv_all(int sock, void* buf, size_t len);
    static bool send_all(int sock, const void* buf, size_t len);
};
