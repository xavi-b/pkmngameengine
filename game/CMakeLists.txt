cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME MODULE_NAME)
string(REPLACE " " "_" MODULE_NAME ${MODULE_NAME})

message("Compiling \"${MODULE_NAME}\"")

add_executable(${MODULE_NAME} "")
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")

install(TARGETS ${MODULE_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR})

# maplibrary
target_link_libraries(${MODULE_NAME} maplibrary)

# pkmnlibrary
target_link_libraries(${MODULE_NAME} pkmnlibrary)

target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} ${SDL2_LIBRARIES})
find_package(SDL2_IMAGE REQUIRED)
find_package(SDL2_TTF REQUIRED)
find_package(SDL2_MIXER REQUIRED)
if(WIN32)
    target_link_directories(${MODULE_NAME} PRIVATE ${SDL2_LIBDIR})
endif()
target_link_libraries(${MODULE_NAME} SDL2_image SDL2_ttf SDL2_mixer)

# boost
find_package(Boost COMPONENTS json locale program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
if(WIN32)
    target_link_directories(${MODULE_NAME} PRIVATE ${Boost_LIBRARY_DIRS})
endif()
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
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/scenes/*.cpp")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/scenes/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# animations
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/animations/*.cpp")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/animations/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# sprites
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sprites/*.cpp")
file(GLOB_RECURSE INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sprites/*.h")

target_sources(${MODULE_NAME}
   PRIVATE
       ${SRC_FILES}
       ${INC_FILES}
)

# wav files
find_program(FFMPEG_PROGRAM ffmpeg)
if(NOT FFMPEG_PROGRAM)
    message(FATAL_ERROR "ffmpeg not found!")
endif()
file(GLOB_RECURSE oggs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/resources/**/*.ogg")
set(wavs)
foreach(ogg ${oggs})
    message("wav file will be generated for ${ogg}")
    set(outputfile "${CMAKE_INSTALL_BINDIR}/${ogg}.wav")
    set(inputfile "${CMAKE_CURRENT_SOURCE_DIR}/${ogg}")
    add_custom_command(
        OUTPUT ${outputfile}
        COMMAND ${FFMPEG_PROGRAM} -y -i ${inputfile} ${outputfile}
        DEPENDS ${inputfile}
        COMMENT "Generating wav for: ${ogg}"
        VERBATIM
    )
    list(APPEND wavs ${outputfile})
endforeach()
add_custom_target(wavs ALL DEPENDS ${wavs})

# resources
file(GLOB_RECURSE RC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/resources/**")
#set_target_properties(${MODULE_NAME} PROPERTIES RESOURCE "${RC_FILES}")
target_sources(${MODULE_NAME}
   PRIVATE
       ${RC_FILES}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_INSTALL_BINDIR})
