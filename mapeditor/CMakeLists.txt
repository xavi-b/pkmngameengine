cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME MODULE_NAME)
string(REPLACE " " "_" MODULE_NAME ${MODULE_NAME})

message("Compiling \"${MODULE_NAME}\"")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.h mainwindow.cpp
        imageviewer.h imageviewer.cpp
        imagewidget.h imagewidget.cpp
        mapperviewer.h mapperviewer.cpp
        mapperwidget.h mapperwidget.cpp
        mappereditor.h mappereditor.cpp
        checkablecombobox.h checkablecombobox.cpp
        levelsmodel.h levelsmodel.cpp
        layersmodel.h layersmodel.cpp
        encounterseditor.h encounterseditor.cpp
        encountersmodel.h encountersmodel.cpp
        encounterdialog.h encounterdialog.cpp
)

qt_add_executable(${MODULE_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# Qt
target_link_libraries(${MODULE_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# maplibrary
target_link_libraries(${MODULE_NAME} PRIVATE maplibrary)

# Binary directory (version.h)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_BINARY_DIR})

set_target_properties(${MODULE_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER ${MODULE_NAME}.${DOMAIN}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${MODULE_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

qt_finalize_executable(${MODULE_NAME})
