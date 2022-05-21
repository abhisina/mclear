Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview5
)

FetchContent_MakeAvailable(Catch2)

add_library(linuxversion_test OBJECT ${CMAKE_SOURCE_DIR}/client/linuxversion.cpp
                                     ${CMAKE_SOURCE_DIR}/client/adjustoomscore.cpp
                                     ${CMAKE_SOURCE_DIR}/client/uname.cpp
                                     ${CMAKE_SOURCE_DIR}/client/memoryinfo.cpp
)
target_include_directories(linuxversion_test INTERFACE ${CMAKE_SOURCE_DIR}/client)
target_link_libraries(linuxversion_test PRIVATE project_options)

add_executable(clienttests clienttests.cpp)
target_link_libraries(clienttests PRIVATE project_options Catch2::Catch2WithMain)
target_link_libraries(clienttests PUBLIC linuxversion_test)

add_test(NAME ClientTests COMMAND clienttests)
