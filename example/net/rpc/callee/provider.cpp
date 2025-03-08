#include <iostream>

#include "lemon/net/rpc/rpc_application.h"
#include "lemon/net/rpc/rpc_provider.h"
#include "lemon/base/config.h"

#include "user.pb.h"

class UserService : public user::UserServiceRpc
{
public:

    bool Login(std::string username, std::string password) {
        // 业务处理
        printf("login: %s, %s\n", username.c_str(), password.c_str());
        return true;
    }

    void Login(::google::protobuf::RpcController*,
               const user::LoginRequest *request,
               user::LoginResponse *response,
               ::google::protobuf::Closure *done) {
        std::string username = request->username();
        std::string password = request->password();

        if (Login(username, password)) {
            response->mutable_rscode()->set_code(0);
            response->mutable_rscode()->set_message("login success");
            response->set_token("123456");
        } else {
            response->mutable_rscode()->set_code(1);
            response->mutable_rscode()->set_message("login failed");
            response->set_token("");
        }

        done->Run();
    }
};

using namespace lemon;
using namespace lemon::rpc;

int main(int argc, char **argv) {
    RpcApplication::init(argc, argv);

    RpcProvider provider(new EventLoop());
    UserService* userService = new UserService();
    provider.registeService(userService);
    
    provider.run();
    delete userService;
    return 0;
}