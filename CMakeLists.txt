cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_VERBOSE_MAKEFILE off)
project(asio_http C CXX)

include_directories(SYSTEM
    deps/asio/asio/include
    include
)
include_directories(
    src
    include/http
)
add_definitions(-DASIO_STANDALONE -DRAPIDJSON_HAS_STDSTRING=1)
file(GLOB SOURCE_FILES src/*.cpp)
add_library(asio_http SHARED ${SOURCE_FILES})
target_link_libraries(asio_http pthread)
set_property(TARGET asio_http PROPERTY CXX_STANDARD 11)
set_property(TARGET asio_http PROPERTY CXX_STANDARD_REQUIRED TRUE)

if(BUILD_EXAMPLE)
    add_executable(example examples/main.cpp)
    target_link_libraries(example asio_http)
    set_property(TARGET example PROPERTY CXX_STANDARD 11)
endif()

install(TARGETS asio_http
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/http DESTINATION include)
