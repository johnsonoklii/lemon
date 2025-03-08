add_executable(provider provider.cpp ../user.pb.cc)
set_target_properties(provider PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}/net/rpc/)
target_include_directories(provider PRIVATE ../)