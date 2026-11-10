cmake_minimum_required(VERSION 3.16)
project(metrodoro VERSION 0.2.3 LANGUAGES CXX)

include(config/build.cmake)
include(config/deps.cmake)

add_subdirectory(modules)
