cmake_minimum_required(VERSION 3.15)

project(Hush)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_OSX_ARCHITECTURES arm64)

set(MACOSX_BUNDLE_ICON_FILE hush.icns)

set(ICNS ${CMAKE_CURRENT_SOURCE_DIR}/icons/hush.icns)

set_source_files_properties(${ICNS} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)
find_package(FLTK 1.4 CONFIG REQUIRED)

set(BUILD_EXAMPLES OFF CACHE BOOL "Disable advobfuscator examples")
set(BUILD_TESTING OFF CACHE BOOL "Disable advobfuscator tests")

add_subdirectory(external/advobfuscator)

add_executable(Hush WIN32 MACOSX_BUNDLE 
    main.cxx 
    database.cxx 
    ${ICNS}
)

target_include_directories(Hush PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Hush PRIVATE 
    fltk::fltk
    advobfuscator
)

# Поддержка потоков для автоочистки буфера обмена
find_package(Threads REQUIRED)
target_link_libraries(Hush PRIVATE Threads::Threads)