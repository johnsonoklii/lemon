file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/lemon/src/ *.cpp )

# message(STATUS "Lemon sources: ${SRCS}")

ADD_LIBRARY(lemon SHARED ${SRCS})

SET_TARGET_PROPERTIES(lemon PROPERTIES OUTPUT_NAME "lemon")
SET_TARGET_PROPERTIES(lemon PROPERTIES CLEAN_DIRECT_OUTPUT 1)