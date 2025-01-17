cmake_minimum_required(VERSION 3.20)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME PROJECT_NAME)
string(REPLACE " " "_" PROJECT_NAME ${PROJECT_NAME})
set(VERSION 0.1.0)
set(VENDOR "xavi-b")
set(DOMAIN "xavi-b.fr")
project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)
configure_file(version.h.in version.h @ONLY)

# C++
set(CMAKE_CXX_STANDARD              20)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS            OFF)
if(UNIX)
    set(CMAKE_CXX_FLAGS                 "-Wall -Wextra -Wpedantic -pthread")
endif()
set(CMAKE_COMPILE_WARNING_AS_ERROR  ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("DEBUG")
    if(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
    endif()
    add_definitions(-DDEBUG)
else()
    message("RELEASE")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/maplibrary)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pkmnlibrary)

# subdirectories
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/maplibrary)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pkmnlibrary)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/game)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mapeditor)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pkmneditor)

add_custom_target(readme SOURCES README.md)
