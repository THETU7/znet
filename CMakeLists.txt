cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

PROJECT(reactor VERSION 0.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /home/thetu/Yang/code/CPP/asyncLoger/)
find_package(Alog REQUIRED)
message(${Alog_INCLUDE_DIRS})

option(ENABLETEST "generate test?" ON)

include_directories(${CMAKE_SOURCE_DIR}/src ${Alog_INCLUDE_DIRS})
add_subdirectory(src)
add_subdirectory(test)

