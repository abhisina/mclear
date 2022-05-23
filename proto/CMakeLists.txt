#find_package(protobuf CONFIG REQUIRED PATHS ${protobuf_BINARY_DIR})
#find_package(gRPC CONFIG REQUIRED PATHS ${grpc_BINARY_DIR})
find_package(Threads REQUIRED)

set(PROTO_FILES
	mcproto/cpuloadinfo.proto
    mcproto/diskinfo.proto
    mcproto/memoryinfo.proto
    mcproto/networkinfo.proto
    mcproto/infoupdate.proto
)

find_program(grpc_cpp_plugin_location NAMES grpc_cpp_plugin PATHS ${grpc_BINARY_DIR} NO_DEFAULT_PATH)
message(STATUS "cpp plugin: ${grpc_cpp_plugin_location}")
add_library(mcproto ${PROTO_FILES})
cmake_policy(PUSH)
cmake_policy(SET CMP0024 OLD)
find_package(protobuf CONFIG REQUIRED PATHS ${protobuf_BINARY_DIR} NO_DEFAULT_PATH)
protobuf_generate(TARGET mcproto LANGUAGE cpp)
protobuf_generate(TARGET mcproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
cmake_policy(POP)
target_link_libraries(mcproto
	PUBLIC
	${grpc++_alts_LIB_DEPENDS}
)
target_include_directories(mcproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${grpc_SOURCE_DIR}/include ${absl_SOURCE_DIR})
