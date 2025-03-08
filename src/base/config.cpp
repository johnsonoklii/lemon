#include "lemon/base/config.h"

#include <cassert>
#include <cstdio>

#include <fstream>
#include <sstream>


using namespace lemon;

void Config::loadConfigFile(const char *config_file) {
    assert(config_file != nullptr);
    
    FILE* file = fopen(config_file, "r");  // 修改为 FILE*

    if (file == nullptr) {
        perror("Failed to open config file");
        return;
    }

    while(!feof(file)) {
        char buf[256] = {0};
        fgets(buf, 256, file);

        std::string read_buf(buf);
        trim(read_buf);

        // 注释
        if (read_buf[0] == '#' || read_buf.empty()) {
            continue;
        }

        std::stringstream ss(read_buf);
        std::string key;
        std::string value;
        getline(ss, key, '=');
        trim(key);

        getline(ss, value, '=');

        // 去掉\n
        int idx = value.find('\n');
        if (idx != -1) {
            value.erase(idx);
        }

        trim(value);
        
        // 去掉“”
        if (value[0] == '"' && value[value.size() - 1] == '"') {
            value.erase(0, 1);
            value.erase(value.size() - 1, 1);
        }

        m_configMap[key] = value;
    }

    fclose(file); 
}

std::string Config::load(const std::string &key) const {
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) {
        return "";
    }
    return it->second;
}

void Config::trim(std::string &str) {
    int index = str.find_first_not_of(' ');
    if (index != -1) {
        str = str.substr(index, str.size() - index);
    }

    index = str.find_last_not_of(' ');
    if (index != -1) {
        str = str.substr(0, index + 1);
    }
}