cmake_minimum_required(VERSION 3.10)
project(arfcc)

file (GLOB SRC
    "*.cpp"
)

add_executable(arfcc ${SRC})
