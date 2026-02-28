cmake_minimum_required(VERSION 3.16)

project(SerialFlow VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Tell AUTOUIC where to find .ui files
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort)

qt_add_executable(SerialFlow
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/serialportmanager.cpp
    src/serialportmanager.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    forms/mainwindow.ui
    forms/settingsdialog.ui
    resources/resources.qrc
)

target_include_directories(SerialFlow PRIVATE src)

target_link_libraries(SerialFlow PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
)

set_target_properties(SerialFlow PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

option(BUILD_TESTS "Build the test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
