#pragma once

#include <string>
#include <unordered_map>

class UserDatabase {
private:
    std::unordered_map<std::string, std::string> users;
    std::string filename;

public:
    UserDatabase(const std::string& dbFile);
    bool load();
    bool isValidUser(const std::string& login) const;
    bool verifyPassword(const std::string& login, const std::string& passwordHash) const;
    std::string getPassword(const std::string& login) const;
};
