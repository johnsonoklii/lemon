#include "lemon/net/rpc/rpc_application.h"

#include <cstdio>
#include <unistd.h>

#include <string>

using namespace lemon;
using namespace lemon::rpc;

Config RpcApplication::m_config;

void ShowArgUsage() {
    printf("format: command -i config_file\n");
}

RpcApplication::RpcApplication(){ }

RpcApplication& RpcApplication::getInstance() {
    static RpcApplication app;
    return app;
}
    
void RpcApplication::init(int argc, char **argv) {
    if(argc < 2) {
        ShowArgUsage();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    const char* config_file = "";
    while ( (c = getopt(argc, argv, "i:")) != -1 ) {
        switch (c) {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                ShowArgUsage();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgUsage();
                exit(EXIT_FAILURE);
            default:
                ShowArgUsage();
                exit(EXIT_FAILURE);
        }
    }

    // 加载配置
    m_config.loadConfigFile(config_file);
}