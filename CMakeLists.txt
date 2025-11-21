cmake_minimum_required(VERSION 3.16)

project(XCanvas VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(QT_SDK_DIR D:/Qt/6.8.3/msvc2022_64 CACHE PATH "QT SDK DIR" FORCE)
list(APPEND CMAKE_PREFIX_PATH ${QT_SDK_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(XCanvas
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        MyGraphicsView.cpp MyGraphicsView.h
        DrawingToolsBar.h DrawingToolsBar.cpp
        Global.h
        BaseDrawingTool.h
        RectDrawingTool.h RectDrawingTool.cpp
        EllipseDrawingTool.h EllipseDrawingTool.cpp
        PolylineDrawingTool.h PolylineDrawingTool.cpp
        SelectDrawingTool.h SelectDrawingTool.cpp
        BaseShape.h
        Shapes.h Shapes.cpp
        PolylineShape.h PolylineShape.cpp
        BaseDrawingTool.cpp
        rulerwidget.h rulerwidget.cpp
        MyMath.h
        BaseShape.cpp
        DXFTranslator.cpp DXFTranslator.h
        EllipseShape.h EllipseShape.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET XCanvas APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(XCanvas SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(XCanvas
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 根据平台区分输出路径
if (WIN32)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/win64")
elseif(APPLE)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/mac")
elseif(UNIX)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/linux")
else()
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/other")
endif()

# 指定输出路径
set_target_properties(XCanvas PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}/Release"
)

target_include_directories(XCanvas PRIVATE
    ${CMAKE_SOURCE_DIR}/3rdparty/libdxfrw/src
)

target_link_directories(XCanvas PRIVATE
    ${CMAKE_SOURCE_DIR}/SDK/lib
)

target_link_libraries(XCanvas PRIVATE
    dxfrw
)

target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.XCanvas)
endif()
set_target_properties(XCanvas PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS XCanvas
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(XCanvas)
endif()
