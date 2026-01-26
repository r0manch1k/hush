cmake_minimum_required(VERSION 3.15)

project(Hush)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_OSX_ARCHITECTURES arm64)

find_package(FLTK 1.4 CONFIG REQUIRED)

add_executable(Hush WIN32 MACOSX_BUNDLE main.cxx database.cxx)

target_include_directories(Hush PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(Hush PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Hush PRIVATE fltk::fltk)