cmake_minimum_required(VERSION 3.16)

project(videoEeg VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LSL REQUIRED HINTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liblsl-1.16.2-Win_amd64/lib/cmake/LSL"
)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS PrintSupport)
find_package(Qt6 REQUIRED COMPONENTS Graphs)
find_package(Qt6 REQUIRED COMPONENTS GraphsWidgets)
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)

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
        qml/EegGraph.qml

        # Common components
        qml/components/common/InfoBanner.qml
        qml/components/common/SuccessBanner.qml
        qml/components/common/DeviceCard.qml
        qml/components/common/NavigationBar.qml
        qml/components/common/SectionHeader.qml

        # EEG components
        qml/components/eeg/ControlSection.qml
        qml/components/eeg/MarkerButton.qml

        # Main components
        qml/components/main/PatientListItem.qml
        qml/components/main/PanelHeader.qml
        qml/components/main/StatusBadge.qml

    SOURCES
        src/viewmodels/AmplifierSetupBackend.h
        src/viewmodels/AmplifierSetupBackend.cpp

        src/viewmodels/EegBackend.h
        src/viewmodels/EegBackend.cpp

        src/managers/amplifiermanager.h
        src/managers/amplifiermanager.cpp

        src/managers/lslstreamreader.h
        src/managers/lslstreamreader.cpp

        src/managers/markermanager.h
        src/managers/markermanager.cpp

        src/globals.h
        src/globals.cpp

        src/models/amplifiermodel.h
        src/models/eegdatamodel.h
        src/models/eegdatamodel.cpp

        src/utils/autoscalemanager.h
        src/utils/autoscalemanager.cpp

    RESOURCES
        notes
)

target_include_directories(appvideoEeg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodels
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
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
        Qt6::Graphs
        Qt6::GraphsWidgets
        Qt6::QuickControls2
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
