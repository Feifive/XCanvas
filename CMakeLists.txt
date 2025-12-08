cmake_minimum_required(VERSION 3.16)

project(XCanvas VERSION 0.1 LANGUAGES CXX)

FILE(GLOB ORIGIN *.cpp *.h)
FILE(GLOB DRAWINGTOOL DrawingTool/*.h DrawingTool/*.cpp)
FILE(GLOB SHAPE Shape/*.h Shape/*.cpp)
FILE(GLOB COMMON Common/*.h Common/*.cpp)

source_group("DrawingTool"  FILES ${DRAWINGTOOL})
source_group("Shape"        FILES ${SHAPE})
source_group("Common"       FILES ${COMMON})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(QT_SDK_DIR /Users/ze/Qt/6.10.1/macos CACHE PATH "QT SDK DIR" FORCE)
list(APPEND CMAKE_PREFIX_PATH ${QT_SDK_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        BottomFloatingToolBar.cpp
        BottomFloatingToolBar.h
        Shape/UndoCommand.h
        Shape/UndoStack.h
        Shape/AddShapeCommand.h
        Shape/RemoveShapeCommand.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(XCanvas
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        MyGraphicsView.cpp MyGraphicsView.h
        DrawingToolsBar.h DrawingToolsBar.cpp
        RulerWidget.h RulerWidget.cpp
        DXFTranslator.cpp DXFTranslator.h
        DrawingTool/DrawingTool.cpp DrawingTool/DrawingTool.h 
        DrawingTool/EllipseTool.cpp DrawingTool/EllipseTool.h 
        DrawingTool/PolylineTool.cpp DrawingTool/PolylineTool.h 
        DrawingTool/RectTool.cpp DrawingTool/RectTool.h 
        DrawingTool/SelectTool.cpp DrawingTool/SelectTool.h
        DrawingTool/TextTool.h DrawingTool/TextTool.cpp
        DrawingTool/CurveTool.cpp DrawingTool/CurveTool.h
        Shape/Shape.cpp Shape/Shape.h 
        Shape/Ellipse.cpp Shape/Ellipse.h 
        Shape/Polyline.cpp Shape/Polyline.h 
        Shape/Shapes.cpp Shape/Shapes.h
        Resource.qrc
        CanvasWidget.h CanvasWidget.cpp
        Common/EventBus.h Common/EventBus.cpp
        Shape/Text.h Shape/Text.cpp
        Shape/Curve.h Shape/Curve.cpp
        Shape/Image.h Shape/Image.cpp
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
    ${PROJECT_SOURCE_DIR}/3rdparty/libdxfrw/src
    ${PROJECT_SOURCE_DIR}/Shape
    ${PROJECT_SOURCE_DIR}/DrawingTool
    ${PROJECT_SOURCE_DIR}/Common
)

if (WIN32)
    set(LIBDXFRW_PATH
        ${PROJECT_SOURCE_DIR}/SDK/lib/x86_64/dxfrw.lib
    )

elseif (APPLE)
    set(LIBDXFRW_PATH
        ${PROJECT_SOURCE_DIR}/SDK/lib/macos/libdxfrw.a
    )

else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

if (NOT EXISTS "${LIBDXFRW_PATH}")
    message(FATAL_ERROR "libdxfrw not found: ${LIBDXFRW_PATH}")
endif()

target_link_libraries(XCanvas PRIVATE "${LIBDXFRW_PATH}")

target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(XCanvas PRIVATE Qt${QT_VERSION_MAJOR}::Core)

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
