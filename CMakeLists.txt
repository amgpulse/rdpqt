cmake_minimum_required(VERSION 3.16)

project(RdpQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic compilation of Qt MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Locate Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Project sources
set(SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/RdpManager.h
    src/RdpManager.cpp
)

add_executable(rdpqt ${SOURCES})

target_link_libraries(rdpqt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
