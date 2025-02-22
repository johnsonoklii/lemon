cmake_minimum_required(VERSION 3.0)

project(lemon)

option(LEMON_BUILD_EXAMPLES "Build Lemon examples" ON)
option(ENABLE_LEMON_DEBUG "Enable LEMON_DEBUG macro" OFF)

set(CXX_FLAGS
 -O2
 -g
 -Wall
 -Wextra
 -Werror
 -std=c++11
 -pthread
)

if(ENABLE_LEMON_DEBUG)
  add_definitions(-DLEMON_DEBUG)
endif()

# FLAGS
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# OUTPUT
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# INCLUDE
include_directories(
  ${PROJECT_SOURCE_DIR}/src/include
  /usr/local/include
  /usr/include
)

# LIBRARIES
link_directories(
    /usr/local/lib
    /usr/lib
    lib
)

if(LEMON_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

add_subdirectory(src)



