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

set(QT_SDK_DIR
    D:/software/Qt6.8.3/6.8.3/msvc2022_64
    CACHE PATH "Qt SDK directory" FORCE
)
list(APPEND CMAKE_PREFIX_PATH ${QT_SDK_DIR})

find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)

# ============================================================
# 第三方库
# ============================================================

add_subdirectory(3rdparty/pdfium)

# ============================================================
# 源文件
# ============================================================

file(GLOB DRAWINGTOOL DrawingTool/*.h DrawingTool/*.cpp)
file(GLOB SHAPE       Shape/*.h Shape/*.cpp)
file(GLOB COMMON      Common/*.h Common/*.cpp)
file(GLOB CANVAS      Canvas/*.h Canvas/*.cpp)
file(GLOB IMPORT_ROOT Import/*.h Import/*.cpp)
file(GLOB IMPORT_UTILS Import/Utils/*.h Import/Utils/*.cpp)
file(GLOB IMPORT_DXF  Import/DXF/*.h Import/DXF/*.cpp)
file(GLOB IMPORT_IMG  Import/Image/*.h Import/Image/*.cpp)
file(GLOB IMPORT_PDF  Import/PDF/*.h Import/PDF/*.cpp)

source_group("DrawingTool"   FILES ${DRAWINGTOOL})
source_group("Shape"         FILES ${SHAPE})
source_group("Common"        FILES ${COMMON})
source_group("Canvas"        FILES ${CANVAS})
source_group("Import"        FILES ${IMPORT_ROOT})
source_group("Import\\Utils" FILES ${IMPORT_UTILS})
source_group("Import\\DXF"   FILES ${IMPORT_DXF})
source_group("Import\\Image" FILES ${IMPORT_IMG})
source_group("Import\\PDF"   FILES ${IMPORT_PDF})

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    BottomFloatingToolBar.cpp BottomFloatingToolBar.h
    MyGraphicsView.cpp MyGraphicsView.h
    DrawingToolsBar.cpp DrawingToolsBar.h
    RulerWidget.cpp RulerWidget.h
    CanvasWidget.cpp CanvasWidget.h

    ${DRAWINGTOOL}
    ${SHAPE}
    ${COMMON}
    ${CANVAS}
    ${IMPORT_ROOT}
    ${IMPORT_DXF}
    ${IMPORT_IMG}
    ${IMPORT_PDF}
    ${IMPORT_UTILS}
)

# ============================================================
# 可执行程序
# ============================================================

if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(XCanvas
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Resource.qrc
    )
else()
    add_executable(XCanvas ${PROJECT_SOURCES})
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

    # NOTE: dxfrw 为源码级第三方，暂时直接 include
    ${PROJECT_SOURCE_DIR}/3rdparty/libdxfrw/src
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
)

# ============================================================
# Windows：运行期 dll 拷贝（pdfium）
# ============================================================

if (WIN32)
    get_target_property(_pdfium_dll thirdparty_pdfium PDFIUM_RUNTIME_DLL)

    add_custom_command(TARGET XCanvas POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_pdfium_dll}"
            $<TARGET_FILE_DIR:XCanvas>
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
