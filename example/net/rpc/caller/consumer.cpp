#include "user.pb.h"

#include "lemon/net/rpc/rpc_application.h"
#include "lemon/net/rpc/rpc_channel.h"

using namespace lemon;
using namespace lemon::rpc;

int main(int argc, char **argv) {
    RpcApplication::init(argc, argv);

    user::UserServiceRpc_Stub stub(new RpcChannel);
    user::LoginRequest request;
    request.set_username("zhangsan");
    request.set_password("123456");

    user::LoginResponse response;
    stub.Login(nullptr, &request, &response, nullptr);
    
    if (response.rscode().code() == 0) {
        printf("rpc login success! tolen is %s\n", response.token().c_str());
    } else {
        printf("rpc login fail! error code is %d, error message is %s\n", response.rscode().code(), response.rscode().message().c_str());
    }

    return 0;
}