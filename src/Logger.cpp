#include "Logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& filename) : logFile(filename) {
    logStream.open(filename, std::ios::app);
}

void Logger::log(LogLevel level, const std::string& message) {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    std::string levelStr;
    switch(level) {
        case LogLevel::INFO: levelStr = "INFO"; break;
        case LogLevel::WARNING: levelStr = "WARNING"; break;
        case LogLevel::ERROR: levelStr = "ERROR"; break;
    }
    
    std::string logEntry = "[" + std::string(timestamp) + "] [" + levelStr + "] " + message;
    
    if (logStream.is_open()) {
        logStream << logEntry << std::endl;
        logStream.flush();
    }
    
    std::cout << logEntry << std::endl;
}
