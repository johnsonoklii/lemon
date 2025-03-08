#include "lemon/net/rpc/rpc_channel.h"
#include "lemon/net/rpc/rpc_application.h"
#include "proto/rpcheader.pb.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace lemon;
using namespace lemon::rpc;
using namespace lemonrpc;

RpcChannel::RpcChannel() {
    Config config = RpcApplication::getConfig();
    m_zkClient = std::unique_ptr<ZkClient>(new ZkClient(&config));
    m_zkClient->Start();
}
    
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done) {
    (void)done;
    const google::protobuf::ServiceDescriptor* serviceDes =  method->service();        
    std::string service_name = serviceDes->name();
    std::string method_name = method->name();       
    
    int args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        controller->SetFailed("request serialize error");
        return;
    }

    RpcHeader header;
    header.set_service_name(service_name);
    header.set_method_name(method_name);
    header.set_args_size(args_size);

    int header_size = 0;
    std::string header_str;
    if (header.SerializeToString(&header_str)) {
        header_size = htonl(header_str.size()); // 网络字节序
    } else {
        controller->SetFailed("header serialize error");
        return;
    }

    std::string send_str;
    send_str.insert(0, std::string((char*)&header_size, 4));
    send_str.append(header_str);
    send_str.append(args_str);

    // std::string ip = RpcApplication::getConfig().load("rpcserverip");
    // int port = atoi(RpcApplication::getConfig().load("rpcserverport").c_str());
    // 注册中心 拿到rpc服务的ip:port
    /*
        分布式部署时，需要考虑负责均衡
    */
    char path[128];
    sprintf(path, "/%s/%s", service_name.c_str(), method_name.c_str());
    std::string zk_host = m_zkClient->GetData(path);
    int idx = zk_host.find(":");
    if (idx == -1) {
        controller->SetFailed("zk get data error");
        return;
    }
    std::string ip = zk_host.substr(0, idx);
    int port = atoi(zk_host.substr(idx + 1, zk_host.size() - idx - 1).c_str());

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        controller->SetFailed("socket error");
        return;
    }

    do {
        if (::connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            printf("connect error\n");
            close(fd);
            exit(EXIT_FAILURE);
        }

        if (-1 == ::send(fd, send_str.c_str(), send_str.size(), 0) ) {
            controller->SetFailed("send error");
            break;
        }

        // recv response
        char buf[1024] = {0};
        int recv_size = 0;
        if (-1 == (recv_size =::recv(fd, buf, sizeof(buf), 0) )) {
            controller->SetFailed("recv error");
            break;
        }

        std::string recv_str(buf, 0, recv_size);

        if (!response->ParseFromString(recv_str)) {
            controller->SetFailed("response parse error");
            break;
        }
    } while(0);

    close(fd);
}