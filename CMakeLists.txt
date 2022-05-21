cmake_minimum_required(VERSION 3.15)

project(mclear CXX)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

Include(FetchContent)
FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG        v1.46.2
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(server)

option(ENABLE_TEST "Turn off to disable tests" ON)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
