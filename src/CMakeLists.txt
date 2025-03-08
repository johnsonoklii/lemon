file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/lemon/src/ *.cpp )
file(GLOB_RECURSE PROTO_SRCS ${PROJECT_SOURCE_DIR}/src/proto *.cc )

# message(STATUS "Lemon sources: ${SRCS}")

ADD_LIBRARY(lemon SHARED ${SRCS} ${PROTO_SRCS})

target_link_libraries(lemon protobuf zookeeper_mt)

SET_TARGET_PROPERTIES(lemon PROPERTIES OUTPUT_NAME "lemon")
SET_TARGET_PROPERTIES(lemon PROPERTIES CLEAN_DIRECT_OUTPUT 1)