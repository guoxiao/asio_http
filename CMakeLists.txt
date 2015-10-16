cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_VERBOSE_MAKEFILE off)
project(asio_http)

include_directories(include)
file(GLOB SOURCE_FILES src/*.cpp src/http/*.cpp)
add_executable(asio_http ${SOURCE_FILES})
target_link_libraries(asio_http pthread http_parser)
set_property(TARGET asio_http PROPERTY CXX_STANDARD 11)

