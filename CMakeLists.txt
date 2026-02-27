cmake_minimum_required(VERSION 3.16)

project(XCanvas VERSION 0.1 LANGUAGES CXX)

# ============================================================
# 基础配置
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================
# Qt 配置
# ============================================================

if(WIN32)
    set(QT_SDK_DIR D:/software/Qt6.8.3/6.8.3/msvc2022_64 CACHE PATH "Qt SDK directory" FORCE)
elseif (APPLE)
    set(QT_SDK_DIR /Users/ze/Qt/6.10.1/macos CACHE PATH "QT SDK DIR" FORCE)
endif ()
list(APPEND CMAKE_PREFIX_PATH ${QT_SDK_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)
find_package(Qt6 REQUIRED COMPONENTS Svg)

# ============================================================
# 第三方库
# ============================================================

add_subdirectory(3rdparty/pdfium)
add_subdirectory(3rdparty/Clipper2Lib)

set(ZSTD_BUILD_STATIC ON CACHE INTERNAL "")
set(ZSTD_BUILD_SHARED OFF CACHE INTERNAL "")
set(ZSTD_BUILD_PROGRAMS OFF CACHE INTERNAL "")
set(ZSTD_BUILD_TESTS OFF CACHE INTERNAL "")
add_subdirectory(3rdparty/zstd/build/cmake EXCLUDE_FROM_ALL)

# ============================================================
# 源文件
# ============================================================

file(GLOB DRAWINGTOOL DrawingTool/*.h DrawingTool/*.cpp)
file(GLOB SHAPE       Shape/*.h Shape/*.cpp)
file(GLOB COMMON      Common/*.h Common/*.cpp)
file(GLOB CANVAS      Canvas/*.h Canvas/*.cpp)
file(GLOB LAYER       Layer/*.h Layer/*.cpp)
file(GLOB IMPORT_ROOT Import/*.h Import/*.cpp)
file(GLOB IMPORT_UTILS Import/Utils/*.h Import/Utils/*.cpp)
file(GLOB IMPORT_DXF  Import/DXF/*.h Import/DXF/*.cpp)
file(GLOB IMPORT_IMG  Import/Image/*.h Import/Image/*.cpp)
file(GLOB IMPORT_PDF  Import/PDF/*.h Import/PDF/*.cpp)
file(GLOB SERIALIZATION  Serialization/*.h Serialization/*.cpp)
file(GLOB CONTROLLER  Controller/*.h Controller/*.cpp)

source_group("DrawingTool"   FILES ${DRAWINGTOOL})
source_group("Shape"         FILES ${SHAPE})
source_group("Common"        FILES ${COMMON})
source_group("Canvas"        FILES ${CANVAS})
source_group("Layer"         FILES ${LAYER})
source_group("Import"        FILES ${IMPORT_ROOT})
source_group("Import\\Utils" FILES ${IMPORT_UTILS})
source_group("Import\\DXF"   FILES ${IMPORT_DXF})
source_group("Import\\Image" FILES ${IMPORT_IMG})
source_group("Import\\PDF"   FILES ${IMPORT_PDF})
source_group("Serialization" FILES ${SERIALIZATION})
source_group("Controller"    FILES ${CONTROLLER})

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    BottomFloatingToolBar.cpp BottomFloatingToolBar.h
    SelectionHudBar.cpp SelectionHudBar.h
    MyGraphicsView.cpp MyGraphicsView.h
    DrawingToolsBar.cpp DrawingToolsBar.h
    RulerWidget.cpp RulerWidget.h
    CanvasWidget.cpp CanvasWidget.h
    SafeDropShadowEffect.h SafeDropShadowEffect.cpp
    XMenu.h XMenu.cpp
    XHoverMenu.cpp XHoverMenu.h
    ColorSwatchButton.cpp ColorSwatchButton.h
    ColorPaletteWidget.cpp ColorPaletteWidget.h

    ${DRAWINGTOOL}
    ${SHAPE}
    ${COMMON}
    ${CANVAS}
    ${LAYER}
    ${IMPORT_ROOT}
    ${IMPORT_DXF}
    ${IMPORT_IMG}
    ${IMPORT_PDF}
    ${IMPORT_UTILS}
    ${SERIALIZATION}
    ${CONTROLLER}
)

# ============================================================
# 可执行程序
# ============================================================

if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(XCanvas
        WIN32
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Resource.qrc
        Layer/LayerManager.h Layer/LayerManager.cpp
        Layer/LayerPanel.h Layer/LayerPanel.cpp
        Common/CSwitchButton.cpp Common/CSwitchButton.h
    )
else()
    add_executable(XCanvas MACOSX_BUNDLE ${PROJECT_SOURCES})
endif()

if (APPLE)
    set_target_properties(XCanvas PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.xcanvas.app"
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING
            ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# ============================================================
# 平台相关：图标
# ============================================================

if (APPLE)
    set(APP_ICON_MACOS "${CMAKE_SOURCE_DIR}/Resource/Icons/XCanvasLogo.icns")
    set_source_files_properties(${APP_ICON_MACOS}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(XCanvas PRIVATE ${APP_ICON_MACOS})
elseif (WIN32)
    target_sources(XCanvas PRIVATE
        "${CMAKE_SOURCE_DIR}/Resource/Icons/XCanvasLogo.rc"
    )
endif()

# ============================================================
# 输出目录
# ============================================================

if (WIN32)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/win64")
elseif (APPLE)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/mac")
else()
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/linux")
endif()

set_target_properties(XCanvas PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}/Release"
)

# ============================================================
# Include 目录
# ============================================================

target_include_directories(XCanvas PRIVATE
    ${PROJECT_SOURCE_DIR}/Shape
    ${PROJECT_SOURCE_DIR}/Canvas
    ${PROJECT_SOURCE_DIR}/Import
    ${PROJECT_SOURCE_DIR}/DrawingTool
    ${PROJECT_SOURCE_DIR}/Common
    ${PROJECT_SOURCE_DIR}/3rdparty/libdxfrw/src
    ${PROJECT_SOURCE_DIR}/3rdparty/zstd/lib
)

# ============================================================
# dxfrw（Imported Library）
# ============================================================

if (WIN32)
    set(LIBDXFRW_PATH "${PROJECT_SOURCE_DIR}/SDK/lib/x86_64/dxfrw.lib")
elseif (APPLE)
    set(LIBDXFRW_PATH "${PROJECT_SOURCE_DIR}/SDK/lib/macos/libdxfrw.a")
endif()

add_library(libdxfrw STATIC IMPORTED)
set_target_properties(libdxfrw PROPERTIES
    IMPORTED_LOCATION "${LIBDXFRW_PATH}"
)

# ============================================================
# 链接
# ============================================================

target_link_libraries(XCanvas PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    libdxfrw
    thirdparty_pdfium
    clipper2lib
    libzstd_static
)
target_link_libraries(XCanvas PRIVATE Qt6::Svg)

# ============================================================
# 运行期动态库拷贝（pdfium）
# ============================================================

if (WIN32)
    get_target_property(_pdfium_dll thirdparty_pdfium PDFIUM_RUNTIME_DLL)
    add_custom_command(TARGET XCanvas POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_pdfium_dll}" $<TARGET_FILE_DIR:XCanvas>
    )
elseif (APPLE)
    get_target_property(_pdfium_dylib thirdparty_pdfium PDFIUM_RUNTIME_DYLIB)
    set(_frameworks_dir "$<TARGET_BUNDLE_CONTENT_DIR:XCanvas>/Frameworks")
    add_custom_command(TARGET XCanvas POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_frameworks_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_pdfium_dylib}" "${_frameworks_dir}"
        COMMAND /usr/bin/install_name_tool -id "@rpath/libpdfium.dylib" "${_frameworks_dir}/libpdfium.dylib"
        VERBATIM
    )
    set_target_properties(XCanvas PROPERTIES
            SKIP_BUILD_RPATH FALSE
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif()

# ============================================================
# 安装 & Qt finalize
# ============================================================

include(GNUInstallDirs)

install(TARGETS XCanvas
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(XCanvas)
endif()
