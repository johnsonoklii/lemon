#ifndef LEMON_RPC_CHANNEL_H
#define LEMON_RPC_CHANNEL_H
#include "lemon/base/zookeeperutil.h"

#include <google/protobuf/service.h>

#include <memory>

namespace lemon {
namespace rpc {

using namespace base;

class RpcChannel: public google::protobuf::RpcChannel {
public:
    RpcChannel();
    
    void CallMethod(const google::protobuf::MethodDescriptor *method,
            google::protobuf::RpcController *controller,
            const google::protobuf::Message *request,
            google::protobuf::Message *response,
            google::protobuf::Closure *done);
private:
    std::unique_ptr<ZkClient> m_zkClient;
};

} // namespace rpc
} // namspace lemon

#endif