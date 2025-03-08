#include "lemon/base/zookeeperutil.h"
#include "lemon/base/config.h"
#include "lemon/base/logger/logger.h"
#include "lemon/net/rpc/rpc_application.h"


#include <semaphore.h>
#include <time.h>

using namespace lemon;
using namespace lemon::rpc;
using namespace lemon::base;
using namespace lemon::log;

void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path,void *watcherCtx) {
    (void)path;(void)watcherCtx;
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient(Config* config)
: m_config(config),m_zhandle(nullptr) {}

ZkClient::~ZkClient() {
    printf("zkclient::~zkclient start...\n");
    if (m_zhandle) {
        zookeeper_close(m_zhandle);
        printf("zkclient::~zkclient zookeeper_close...\n");
    }
    printf("zkclient::~zkclient end...\n");
}

void ZkClient::Start() {
    if (m_config == nullptr) {
        LOG_FATAL("ZkClient::Start(): config is null");
    }
    std::string ip = m_config->load("zookeeperip");
    std::string port = m_config->load("zookeeperport");
    std::string host = ip + ":" + port;

    m_zhandle = zookeeper_init(host.c_str(), global_watcher, 30000, 0, nullptr, 0);
    if (m_zhandle == nullptr) {
        printf("zookeeper_init error\n");
        exit(EXIT_FAILURE);
    }

    // zookeeper_init是异步进行连接，通过global_watcher回调函数判断是否连接成功
    // 信号量需要等待global_watcher给出信号
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);

    printf("zookeeper_init success\n");
}

void ZkClient::Create(const char* path, const char* data, int datalen, int state) {
    // int flag = zoo_aexists(m_zhandle, path, 0, zktest_stat_completion, "aexists");
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE) {
        char buffer[512];
        flag = zoo_create(m_zhandle, path, data, datalen,
                    &ZOO_OPEN_ACL_UNSAFE, state, buffer, sizeof(buffer) - 1);
        if (flag) {
            printf("zoo_create error, flag = %d\n", flag);
            zookeeper_close(m_zhandle);
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZkClient::GetData(const char* path) {
    char buf[128];
    int len = sizeof(buf);

    int flag = zoo_get(m_zhandle, path, 0, buf, &len, nullptr);
    if (flag != ZOK) {
        return "";
    }
    return buf;
}

std::vector<std::string> ZkClient::GetChildren(const char* path) {
    std::vector<std::string> childrenList;

    struct String_vector children;
    int ret = zoo_get_children(m_zhandle, path, 1, &children);

    if (ret == ZOK) {
        childrenList.reserve(children.count);
        for (int i = 0; i < children.count; i++) {
            childrenList.emplace_back(children.data[i]);
        }
        deallocate_String_vector(&children);
    } else {
        LOG_ERROR("zoo_get_children error, ret = %d", ret);
    }

    return childrenList;
}