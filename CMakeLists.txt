cmake_minimum_required(VERSION 3.6)
project(cdb)

set(CMAKE_CXX_STANDARD 11)
link_libraries(ws2_32 wsock32)
include_directories(asio)
set(SOURCE_FILES main.cpp cdb.cpp cdb.h)
add_executable(cdb ${SOURCE_FILES})
add_executable(cserver cserver.cpp)