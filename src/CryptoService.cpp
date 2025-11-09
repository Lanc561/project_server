#include "CryptoService.h"
#include <openssl/evp.h>
#include <random>
#include <sstream>
#include <iomanip>

std::string CryptoService::computeMD5(const std::string& data) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (!context) {
        throw std::runtime_error("Failed to create MD5 context");
    }
    
    if (EVP_DigestInit_ex(context, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to initialize MD5 digest");
    }
    
    if (EVP_DigestUpdate(context, data.c_str(), data.length()) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to update MD5 digest");
    }
    
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    if (EVP_DigestFinal_ex(context, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to finalize MD5 digest");
    }
    
    EVP_MD_CTX_free(context);
    
    std::stringstream ss;
    for (unsigned int i = 0; i < digest_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

uint64_t CryptoService::generateSalt() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

std::string CryptoService::saltToHexString(uint64_t salt) {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << salt;
    return ss.str();
}
