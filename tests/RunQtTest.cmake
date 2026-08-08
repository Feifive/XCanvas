if(NOT DEFINED TEST_EXECUTABLE OR NOT EXISTS "${TEST_EXECUTABLE}")
    message(FATAL_ERROR "Test executable does not exist: ${TEST_EXECUTABLE}")
endif()

if(NOT DEFINED QT_RUNTIME_DIR OR NOT IS_DIRECTORY "${QT_RUNTIME_DIR}")
    message(FATAL_ERROR "Qt runtime directory does not exist: ${QT_RUNTIME_DIR}")
endif()

set(ENV{PATH} "${QT_RUNTIME_DIR};$ENV{PATH}")

execute_process(
    COMMAND "${TEST_EXECUTABLE}"
    RESULT_VARIABLE test_result
)

if(NOT test_result STREQUAL "0")
    message(FATAL_ERROR "Test exited with code ${test_result}")
endif()
