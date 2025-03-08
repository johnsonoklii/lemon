add_executable(consumer consumer.cpp ../user.pb.cc)
set_target_properties(consumer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/net/rpc/)
target_include_directories(consumer PRIVATE ../)