#ifndef LEMON_RPC_PROVIDER_H
#define LEMON_RPC_PROVIDER_H

#include "lemon/net/eventloop.h"

#include <google/protobuf/service.h>

#include <unordered_map>
#include <memory>

namespace lemon {
namespace rpc {

using namespace lemon::net;

class RpcProvider {
public:
    RpcProvider(EventLoop* loop);
    ~RpcProvider() = default;

    void registeService(google::protobuf::Service *service);
    void run();

    struct RpcParseState {
        int header_size;
        int args_size;
    };

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void sendRpcResponse(const TcpConnectionPtr&, google::protobuf::Message*);

private:
    struct ServiceInfo {
        google::protobuf::Service* m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    std::shared_ptr<EventLoop> m_loop;
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
};

} // namespace rpc
} // namespace lemon

#endif