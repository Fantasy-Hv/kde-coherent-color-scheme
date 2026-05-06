cmake_minimum_required(VERSION 4.1)
project(dy_config_loader)

set(CMAKE_CXX_STANDARD 20)

add_executable(color-config-injector lib/main.cpp
        lib/ColorReader.h
        lib/ConfigLoader.h
        lib/Logger.h
        lib/DycolorConfigKeys.h
        lib/ioutils.h
        lib/KdeColorsLoader.h)
