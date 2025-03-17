#include "lemon/net/rpc/rpc_provider.h"
#include "lemon/net/rpc/rpc_application.h"
#include "lemon/net/tcp_server.h"
#include "lemon/net/tcpconnection.h"
#include "lemon/base/logger/logger.h"
#include "lemon/base/zookeeperutil.h"
#include "proto/rpcheader.pb.h"

#include <google/protobuf/descriptor.h> 

using namespace lemon;
using namespace lemon::rpc;
using namespace lemon::log;
using namespace lemonrpc;

RpcProvider::RpcProvider(EventLoop* loop): m_loop(loop) {

}

void RpcProvider::registeService(google::protobuf::Service *service) {
    ServiceInfo serviceInfo;
    const google::protobuf::ServiceDescriptor* pServiceDesc = service->GetDescriptor();
    std::string serviceName = pServiceDesc->name();

    LOG_INFO("RpcProvider::registeService() serviceName:%s\n", serviceName.c_str());

    int methodCount = pServiceDesc->method_count();
    for (int i = 0; i < methodCount; ++i) {
        const google::protobuf::MethodDescriptor* pMethodDesc = pServiceDesc->method(i);
        std::string methodName = pMethodDesc->name();
        serviceInfo.m_methodMap[methodName] = pMethodDesc;
        LOG_INFO("RpcProvider::registeService() methodName:%s\n", methodName.c_str());
    }
    
    serviceInfo.m_service = service;
    m_serviceMap[serviceName] = serviceInfo;
}

void RpcProvider::run() {
    Config config = RpcApplication::getConfig();

    // FIXME: 判断是否存在
    std::string ip = config.load("rpcserverip");
    int port = atoi(config.load("rpcserverport").c_str());

    InetAddress addr(ip, port);
    TcpServer server(m_loop.get(), addr, "RpcProvider");
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1,
                                                     std::placeholders::_2, std::placeholders::_3));
    
    server.setThreadNum(2);

    // 注册中心
    /*
        FIXME: 如果多个rpc服务实例，应该创建多个instance
        比如，现在要部署3个登录服务，就需要添加3个service/login_service/{instance_id}节点，
        其中，instance_id可以使用以下方式生成
        1. ip+port、uuid、雪花算法
        2. 手动指定
        3. Kubernetes的pod名
        4. zk的临时顺序节点
    */
    ZkClient zkClient(&config);
    zkClient.Start();
    for (auto &service : m_serviceMap) {
        std::string serviceNode = "/" + service.first;
        zkClient.Create(serviceNode.c_str(), nullptr, 0, 0);
        for (auto &method : service.second.m_methodMap) {
            std::string methodNode = serviceNode + "/" + method.first;
            char buf[64];
            sprintf(buf, "%s:%d", ip.c_str(), port);
            zkClient.Create(methodNode.c_str(), buf, strlen(buf), ZOO_EPHEMERAL);
        }
    }

    LOG_INFO("RpcProvider::run() ip:%s, port:%d\n", ip.c_str(), port);

    server.start();
    m_loop->loop();
}

void RpcProvider::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setContext("RpcParseState", std::make_shared<RpcParseState>());
    }else {
        conn->shutdown();
    }
}

void RpcProvider::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp) {
    // FIXME: 不应该一次性读完，可能有半包，应该先4个字节读出来，然后根据header_size再读剩下的数据
    // conn需要保存读取的状态, header_size, args_size, is_parsing_header_size 
    // 每次需要判断长度是否完整

    auto state = conn->getContext<RpcParseState>("RpcParseState");
    if (!state) {
        state = std::make_shared<RpcParseState>();
        conn->setContext("RpcParseState", state);
    }

    // FIXME: 还是有问题，参考http的解析，应该使用状态机的方式
    while(true) {
        if (state->header_size == 0) {
            // 读取header_size
            if (buffer->readableBytes() < 4) return;
            state->header_size = ntohl(*reinterpret_cast<const uint32_t*>(buffer->peek()));
            buffer->retrieve(4);
        } else {
            // 读取header_str
            if (buffer->readableBytes() < (size_t)state->header_size) return;
            std::string header_str = buffer->retrieveAsString(state->header_size);

            RpcHeader header;
            std::string service_name;
            std::string method_name;

            if (header.ParseFromString(header_str)) {
                service_name = header.service_name();
                method_name = header.method_name();
                state->args_size = header.args_size();
            } else {
                LOG_ERROR("RpcProvider::onMessage() parse header error!");
                return; // FIXME: 如果出错，应该关闭连接？
            }

            // 读取args_str
            if (buffer->readableBytes() < (size_t)state->args_size) return;
            std::string args_str = buffer->retrieveAsString(state->args_size);

            printf("service_name2: %s\n", service_name.c_str());
            printf("method_name2: %s\n", method_name.c_str());
            printf("args_str2: %s\n", args_str.c_str());

            auto it = m_serviceMap.find(service_name);
            if (it == m_serviceMap.end()) {
                LOG_ERROR("RpcProvider::onMessage() service_name:%s not exist!", service_name.c_str());
                return;
            }

            auto mit = it->second.m_methodMap.find(method_name);
            if (mit == it->second.m_methodMap.end()) {
                LOG_ERROR("RpcProvider::onMessage() service_name:%s method_name:%s not exist!", service_name.c_str(), method_name.c_str());
                return;
            }

            google::protobuf::Service* service = it->second.m_service;
            const google::protobuf::MethodDescriptor* method = mit->second;
            
            // FIXME: request需要手动释放内存吗
            google::protobuf::Message* request = service->GetRequestPrototype(method).New();
            if (!request->ParseFromString(args_str)) {
                LOG_ERROR("RpcProvider::onMessage() service_name:%s method_name:%s parse args error!", service_name.c_str(), method_name.c_str());
                return;
            }

            google::protobuf::Message* response = service->GetResponsePrototype(method).New();
            google::protobuf::Closure* done = google::protobuf::NewCallback<RpcProvider
                                                                            , const TcpConnectionPtr&
                                                                            , google::protobuf::Message*>
                                                                            (this, &RpcProvider::sendRpcResponse, conn, response);


            service->CallMethod(method, nullptr, request, response, done);

            
            state->header_size = 0;
            state->args_size = 0;
        }
    }
}


void RpcProvider::sendRpcResponse(const TcpConnectionPtr& conn, google::protobuf::Message* response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        conn->send(response_str);
    } else {
        printf("serialize response error\n");
    }
    
    conn->shutdown();
}
