cmake_minimum_required(VERSION 3.16)

project(videoEeg VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LSL REQUIRED HINTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liblsl-1.16.2-Win_amd64/lib/cmake/LSL"
)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS PrintSupport)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appvideoEeg
    src/main.cpp
)

qt_policy(SET QTP0004 NEW)

qt_add_qml_module(appvideoEeg
    URI videoEeg
    VERSION 1.0
    QML_FILES
        qml/main.qml
        qml/MainWindow.qml
        qml/AmplifierSetupWindow.qml
        qml/EegWindow.qml

    SOURCES
        src/viewmodels/AmplifierSetupBackend.h
        src/viewmodels/AmplifierSetupBackend.cpp

        src/viewmodels/eegviewmodel.h
        src/viewmodels/eegviewmodel.cpp

        src/managers/amplifiermanager.h
        src/managers/amplifiermanager.cpp

        src/managers/lslstreamreader.h
        src/managers/lslstreamreader.cpp

        src/globals.h
        src/globals.cpp

        src/models/amplifiermodel.h

        src/viewmodels/eegplotitem.h
        src/viewmodels/eegplotitem.cpp

        3rdparty/QCustomPlot/qcustomplot.h
        3rdparty/QCustomPlot/qcustomplot.cpp

        RESOURCES
            notes
)

target_include_directories(appvideoEeg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodels
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers
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
        Qt6::Widgets
        Qt6::PrintSupport
        LSL::lsl
)
target_link_libraries(appvideoEeg PRIVATE Qt6::Core)
target_link_libraries(appvideoEeg PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS appvideoEeg
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
