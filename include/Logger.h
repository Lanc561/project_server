#pragma once

#include <string>
#include <fstream>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream logStream;
    std::string logFile;

public:
    Logger(const std::string& filename);
    void log(LogLevel level, const std::string& message);
};
