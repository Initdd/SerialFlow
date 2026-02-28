cmake_minimum_required(VERSION 3.16)

project(SerialFlowTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Test SerialPort)

qt_add_executable(tst_serialportmanager
    tst_serialportmanager.cpp
    ../src/serialportmanager.cpp
    ../src/serialportmanager.h
)

target_include_directories(tst_serialportmanager PRIVATE ../src)

target_link_libraries(tst_serialportmanager PRIVATE
    Qt6::Test
    Qt6::SerialPort
)

enable_testing()
add_test(NAME tst_serialportmanager COMMAND tst_serialportmanager)
