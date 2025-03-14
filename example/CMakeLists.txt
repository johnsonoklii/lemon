set(BIN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

link_libraries(lemon)

# logger
file(GLOB LOGGER_SRCS base/logger/*.cpp)
foreach(LOGGER_SRC_FILE ${LOGGER_SRCS})
    get_filename_component(LOGGER_BASE_NAME ${LOGGER_SRC_FILE} NAME_WE)  
    add_executable(${LOGGER_BASE_NAME} ${LOGGER_SRC_FILE})
    set_target_properties(${LOGGER_BASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/base/logger)
endforeach()

# base
file(GLOB BASE_SRCS base/*.cpp)
foreach(BASE_SRC_FILE ${BASE_SRCS})
    get_filename_component(BASE_BASE_NAME ${BASE_SRC_FILE} NAME_WE)  
    add_executable(${BASE_BASE_NAME} ${BASE_SRC_FILE})
    set_target_properties(${BASE_BASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/base)
endforeach()

# net
file(GLOB NET_SRCS net/*.cpp)
foreach(NET_SRC_FILE ${NET_SRCS})
    get_filename_component(NET_BASE_NAME ${NET_SRC_FILE} NAME_WE)  
    add_executable(${NET_BASE_NAME} ${NET_SRC_FILE})
    set_target_properties(${NET_BASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/net)
endforeach()

# rpc
add_subdirectory(net/rpc/callee)
add_subdirectory(net/rpc/caller)

# file(GLOB_RECURSE RPC_SRCS net/rpc/*.cpp)
# foreach(RPC_SRC_FILE ${RPC_SRCS})
#     get_filename_component(RPC_BASE_NAME ${RPC_SRC_FILE} NAME_WE)  
#     add_executable(${RPC_BASE_NAME} ${RPC_SRC_FILE})
#     set_target_properties(${RPC_BASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/net/rpc)
# endforeach()