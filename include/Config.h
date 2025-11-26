// Config.h
#pragma once

#include <string>

class Config {
private:
    std::string userDbFile;
    std::string logFile;
    int port;

public:
    bool parseCommandLine(int argc, char* argv[]);
    void printHelp();
    std::string getUserDbFile() const;
    std::string getLogFile() const;
    int getPort() const;
};
