cmake_minimum_required(VERSION 3.16)
project(RSICV_Tomasulo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS " -Ofast")

add_executable(code main.cpp)

