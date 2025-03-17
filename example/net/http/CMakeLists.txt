add_executable(http_server http_server.cpp)

# 设置调试信息标志
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

set_target_properties(http_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/net/http/)