#ifndef LEMON_RPC_APPLICATION_H
#define LEMON_RPC_APPLICATION_H

#include "lemon/base/config.h"

namespace lemon {
namespace rpc {

class RpcApplication {
public:
    static RpcApplication& getInstance();
    
    static void init(int argc, char **argv);
    static Config& getConfig() { return m_config; }

private:
    RpcApplication();
    RpcApplication(const RpcApplication&) = delete;
    RpcApplication(const RpcApplication&&) = delete;
    RpcApplication& operator=(const RpcApplication&) = delete;

    static Config m_config;
};

} // namespace rpc
} // namespace lemon

#endif