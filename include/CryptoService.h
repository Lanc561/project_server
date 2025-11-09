#pragma once

#include <string>
#include <cstdint>

class CryptoService {
public:
    static std::string computeMD5(const std::string& data);
    static uint64_t generateSalt();
    static std::string saltToHexString(uint64_t salt);
};
