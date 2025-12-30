cmake_minimum_required(VERSION 3.16)

project(Clipper2Lib LANGUAGES CXX)

add_library(clipper2lib STATIC)

target_sources(clipper2lib PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.engine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.offset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.rectclip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.triangulation.cpp
)

target_include_directories(clipper2lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(clipper2lib PUBLIC cxx_std_17)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.engine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.offset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.rectclip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/clipper.triangulation.cpp
)
