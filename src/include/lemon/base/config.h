#ifndef LEMON_CONFIG_H
#define LEMON_CONFIG_H

#include <unordered_map>
#include <string>

namespace lemon {

class Config {
public:
    void loadConfigFile(const char *config_file);
    std::string load(const std::string &key) const;

private:
    void trim(std::string &str);

private:
    std::unordered_map<std::string, std::string> m_configMap;
};

} // namespace mrpc

#endif