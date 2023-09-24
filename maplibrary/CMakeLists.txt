cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME MODULE_NAME)
string(REPLACE " " "_" MODULE_NAME ${MODULE_NAME})

message("Compiling \"${MODULE_NAME}\"")

add_library(${MODULE_NAME} "")

install(TARGETS ${MODULE_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# boost
find_package(Boost COMPONENTS json REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${MODULE_NAME} ${Boost_LIBRARIES})

# VARS
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# global
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# tiles
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tiles/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tiles/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# layers
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/layers/*.cpp")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/layers/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)
