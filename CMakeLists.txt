cmake_minimum_required(VERSION 3.10)
project(arfcc)

set (CMAKE_CXX_STANDARD 17)

file (GLOB SRC
    "*.cpp"
)

add_executable(arfcc ${SRC})
