cmake_minimum_required(VERSION 3.16)
project(LaserTagApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(LaserTagApp main.cpp)
target_link_libraries(LaserTagApp PRIVATE Qt6::Widgets)