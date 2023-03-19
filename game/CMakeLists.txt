cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME MODULE_NAME)
string(REPLACE " " "_" MODULE_NAME ${MODULE_NAME})

message("Compiling \"${MODULE_NAME}\"")

add_executable(${MODULE_NAME} "")
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} ${SDL2_LIBRARIES})
find_package(SDL2_IMAGE REQUIRED)
find_package(SDL2_TTF REQUIRED)
target_link_libraries(${MODULE_NAME} SDL2_image SDL2_ttf SDL2_mixer)

# boost
find_package(Boost COMPONENTS locale program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${MODULE_NAME} ${Boost_LIBRARIES})

# VARS
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

# global
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# tiles
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tiles/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tiles/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# battles
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/battles/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/battles/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# entities
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/entities/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/entities/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# scenes
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/scenes/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/scenes/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# resources
file(GLOB_RECURSE RC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/resources/**")
#set_target_properties(${MODULE_NAME} PROPERTIES RESOURCE "${RC_FILES}")
target_sources(${MODULE_NAME}
   PRIVATE
       ${RC_FILES}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
