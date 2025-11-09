#include "Config.h"
#include <iostream>

bool Config::parseCommandLine(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        printHelp();
        return false;
    }
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-d" && i + 1 < argc) {
            userDbFile = argv[++i];
        }
        else if (arg == "-l" && i + 1 < argc) {
            logFile = argv[++i];
        }
        else if (arg == "-p" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
        else if (arg == "-h" || arg == "--help") {
            printHelp();
            return false;
        }
    }
    
    if (userDbFile.empty() || logFile.empty() || port == 0) {
        std::cerr << "Ошибка: Не все обязательные параметры указаны" << std::endl;
        printHelp();
        return false;
    }
    
    return true;
}

void Config::printHelp() {
    std::cout << "Использование: server -d <файл_базы> -l <файл_журнала> -p <порт>" << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  -d <файл>    Файл базы пользователей" << std::endl;
    std::cout << "  -l <файл>    Файл журнала" << std::endl;
    std::cout << "  -p <порт>    Порт сервера" << std::endl;
    std::cout << "  -h, --help   Показать эту справку" << std::endl;
}

std::string Config::getUserDbFile() const { return userDbFile; }
std::string Config::getLogFile() const { return logFile; }
int Config::getPort() const { return port; }
