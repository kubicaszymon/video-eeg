cmake_minimum_required(VERSION 3.16)

project(videoEeg VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LSL REQUIRED HINTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liblsl-1.16.2-Win_amd64/lib/cmake/LSL"
)

message(STATUS "LSL_FOUND: ${LSL_FOUND}")
message(STATUS "LSL_INCLUDE_DIRS: ${LSL_INCLUDE_DIRS}")
message(STATUS "LSL_LIBRARIES: ${LSL_LIBRARIES}")

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appvideoEeg
    src/main.cpp
)

qt_add_qml_module(appvideoEeg
    URI videoEeg
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        SOURCES include/videoeegapp.h src/videoeegapp.cpp
        SOURCES include/amplifiermanager.h src/amplifiermanager.cpp
        SOURCES include/amplifierparser.h
        QML_FILES qml/EegGraphsWindow.qml
        SOURCES
        SOURCES include/eegviewmodel.h src/eegviewmodel.cpp
        QML_FILES qml/EegChannelGraph.qml
        SOURCES include/lslstreamreader.h src/lslstreamreader.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appvideoEeg PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appvideoEeg
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appvideoEeg
    PRIVATE 
        Qt6::Quick
        LSL::lsl
)
target_link_libraries(appvideoEeg PRIVATE Qt6::Core)
target_link_libraries(appvideoEeg PRIVATE Qt6::Core)
target_link_libraries(appvideoEeg PRIVATE Qt6::Core)
target_link_libraries(appvideoEeg PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS appvideoEeg
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
