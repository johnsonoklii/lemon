set(BIN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

link_libraries(lemon)

# logger
file(GLOB LOGGER_SRCS base/logger/*.cpp)
foreach(LOGGER_SRC_FILE ${LOGGER_SRCS})
    get_filename_component(LOGGER_BASE_NAME ${LOGGER_SRC_FILE} NAME_WE)  
    add_executable(${LOGGER_BASE_NAME} ${LOGGER_SRC_FILE})
    set_target_properties(${LOGGER_BASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/base/logger)
endforeach()

# co
file(GLOB CO_SRCS base/co/*.cpp)
foreach(CO_SRC_FILE ${CO_SRCS})
    get_filename_component(CO_BASE_NAME ${CO_SRC_FILE} NAME_WE)  
    add_executable(${CO_BASE_NAME} ${CO_SRC_FILE})
    set_target_properties(${CO_BASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/base/co)
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
