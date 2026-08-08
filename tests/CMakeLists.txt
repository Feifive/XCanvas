add_executable(tst_CanvasCamera
    tst_CanvasCamera.cpp
    ../Canvas/CanvasCamera.cpp
    ../Canvas/CanvasCamera.h
)

target_include_directories(tst_CanvasCamera PRIVATE
    ${PROJECT_SOURCE_DIR}/Canvas
    ${PROJECT_SOURCE_DIR}/Common
)

target_link_libraries(tst_CanvasCamera PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
)

if(WIN32)
    add_test(
        NAME CanvasCamera
        COMMAND ${CMAKE_COMMAND}
            "-DTEST_EXECUTABLE=$<TARGET_FILE:tst_CanvasCamera>"
            "-DQT_RUNTIME_DIR=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RunQtTest.cmake"
    )
else()
    add_test(NAME CanvasCamera COMMAND tst_CanvasCamera)
endif()

add_executable(tst_CanvasViewportContract
    tst_CanvasViewportContract.cpp
    ../Canvas/ICanvasViewport.h
)

target_include_directories(tst_CanvasViewportContract PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_CanvasViewportContract PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Test
)

if(WIN32)
    add_test(
        NAME CanvasViewportContract
        COMMAND ${CMAKE_COMMAND}
            "-DTEST_EXECUTABLE=$<TARGET_FILE:tst_CanvasViewportContract>"
            "-DQT_RUNTIME_DIR=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RunQtTest.cmake"
    )
else()
    add_test(NAME CanvasViewportContract COMMAND tst_CanvasViewportContract)
endif()

add_executable(tst_CanvasView
    tst_CanvasView.cpp
    ../Canvas/CanvasView.cpp
    ../Canvas/CanvasView.h
    ../Canvas/CanvasCamera.cpp
    ../Canvas/CanvasCamera.h
    ../Canvas/RulerRenderer.cpp
    ../Canvas/RulerRenderer.h
    ../Canvas/ICanvasViewport.h
)

target_include_directories(tst_CanvasView PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Canvas
    ${PROJECT_SOURCE_DIR}/Common
)

target_link_libraries(tst_CanvasView PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
)

if(WIN32)
    add_test(
        NAME CanvasView
        COMMAND ${CMAKE_COMMAND}
            "-DTEST_EXECUTABLE=$<TARGET_FILE:tst_CanvasView>"
            "-DQT_RUNTIME_DIR=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RunQtTest.cmake"
    )
else()
    add_test(NAME CanvasView COMMAND tst_CanvasView)
endif()

add_executable(tst_TextEditCommit
    tst_TextEditCommit.cpp
    ../Shape/Shape.cpp
    ../Shape/Shape.h
    ../Shape/ShapeText.cpp
    ../Shape/ShapeText.h
    ../Shape/ShapeManager.cpp
    ../Shape/ShapeManager.h
    ../Shape/EditTextCommand.h
    ../Common/MyMath.cpp
    ../Common/MyMath.h
    ../Controller/TextEditCommit.h
)

target_include_directories(tst_TextEditCommit PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Shape
    ${PROJECT_SOURCE_DIR}/Common
)

target_link_libraries(tst_TextEditCommit PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
)

if(WIN32)
    add_test(
        NAME TextEditCommit
        COMMAND ${CMAKE_COMMAND}
            "-DTEST_EXECUTABLE=$<TARGET_FILE:tst_TextEditCommit>"
            "-DQT_RUNTIME_DIR=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RunQtTest.cmake"
    )
else()
    add_test(NAME TextEditCommit COMMAND tst_TextEditCommit)
endif()

add_executable(tst_ShapeRenderer
    tst_ShapeRenderer.cpp
    ../Controller/ShapeRenderer.cpp
    ../Controller/ShapeRenderer.h
    ../Shape/Shape.cpp
    ../Shape/Shape.h
    ../Shape/ShapeImage.cpp
    ../Shape/ShapeImage.h
    ../Shape/ShapeVector.cpp
    ../Shape/ShapeVector.h
    ../Common/MyMath.cpp
    ../Common/MyMath.h
)

target_include_directories(tst_ShapeRenderer PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Shape
    ${PROJECT_SOURCE_DIR}/Common
)

target_link_libraries(tst_ShapeRenderer PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
)

if(WIN32)
    add_test(
        NAME ShapeRenderer
        COMMAND ${CMAKE_COMMAND}
            "-DTEST_EXECUTABLE=$<TARGET_FILE:tst_ShapeRenderer>"
            "-DQT_RUNTIME_DIR=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RunQtTest.cmake"
    )
else()
    add_test(NAME ShapeRenderer COMMAND tst_ShapeRenderer)
endif()
