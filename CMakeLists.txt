cmake_minimum_required(VERSION 3.6)
project(smartHouse_client)

set(CMAKE_CXX_STANDARD 11)
find_package(Boost COMPONENTS system thread REQUIRED)
link_libraries(Xft rt pthread X11 nana boost_system boost_thread)
set(SOURCE_FILES main.cpp web/client_http.hpp)
add_executable(smartHouse_client ${SOURCE_FILES})