cmake_minimum_required(VERSION 4.1)
project(dy_config_loader)

set(CMAKE_CXX_STANDARD 20)

add_executable(dy_config_loader lib/main.cpp
        lib/ColorReader.cpp
        lib/ColorReader.h
        lib/ConfigLoader.cpp
        lib/ConfigLoader.h
        lib/Logger.cpp
        lib/Logger.h)
