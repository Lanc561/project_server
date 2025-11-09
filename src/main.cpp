#include "Config.h"
#include "Server.h"

int main(int argc, char* argv[]) {
    Config config;
    if (!config.parseCommandLine(argc, argv)) {
        return 1;
    }
    
    Server server(config);
    server.start();
    
    return 0;
}
