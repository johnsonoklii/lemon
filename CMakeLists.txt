cmake_minimum_required(VERSION 3.0)

project(lemon)

set(CXX_FLAGS
 -O2
 -g
 -Wall
 -Wextra
 -Werror
 -std=c++11
 -pthread
)

# FLAGS
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# OUTPUT
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# INCLUDE
include_directories(
  ${PROJECT_SOURCE_DIR}
  /usr/local/include
  /usr/include
)

# LIBRARIES
link_directories(
    /usr/local/lib
    /usr/lib
)

add_subdirectory(lemon/src)
add_subdirectory(lemon/example)
