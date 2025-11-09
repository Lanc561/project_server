#include "UserDatabase.h"
#include <fstream>

UserDatabase::UserDatabase(const std::string& dbFile) : filename(dbFile) {}

bool UserDatabase::load() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string login = line.substr(0, pos);
            std::string password = line.substr(pos + 1);
            users[login] = password;
        }
    }
    file.close();
    return true;
}

bool UserDatabase::isValidUser(const std::string& login) const {
    return users.find(login) != users.end();
}

bool UserDatabase::verifyPassword(const std::string& login, const std::string& passwordHash) const {
    auto it = users.find(login);
    if (it == users.end()) {
        return false;
    }
    return it->second == passwordHash;
}

std::string UserDatabase::getPassword(const std::string& login) const {
    auto it = users.find(login);
    return it != users.end() ? it->second : "";
}
