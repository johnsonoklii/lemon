#ifndef LEMON_ZOOKEEPERUTIL_H
#define LEMON_ZOOKEEPERUTIL_H

#define THREADED
#include "lemon/base/config.h"

#include <zookeeper/zookeeper.h>

#include <string>
#include <vector>

namespace lemon {
namespace base {

class ZkClient {
public:
    ZkClient(Config* config);
    ~ZkClient();
    void Start();
    void Create(const char* path, const char* data, int datalen, int state = 0);
    std::string GetData(const char* path);
    std::vector<std::string> GetChildren(const char* path);
private:
    Config* m_config;
    zhandle_t* m_zhandle;
};

} // namespace base
} // namespace mrpc

#endif