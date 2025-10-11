cmake_minimum_required(VERSION 3.16)
project(SerialFlow VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/serialportmanager.cpp
    src/settingsdialog.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/serialportmanager.h
    src/settingsdialog.h
)

# Resource files
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
