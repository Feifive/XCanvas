add_library(thirdparty_pdfium INTERFACE)

if (WIN32)
    set(PDFium_DIR
        ${CMAKE_CURRENT_LIST_DIR}/win
    )
elseif (APPLE)
    set(PDFium_DIR
        ${CMAKE_CURRENT_LIST_DIR}/macos
    )
endif()

find_package(PDFium CONFIG REQUIRED)

target_link_libraries(thirdparty_pdfium INTERFACE
    PDFium::pdfium
)

if (WIN32)
    get_target_property(_pdfium_runtime PDFium::pdfium IMPORTED_LOCATION)
    set_property(TARGET thirdparty_pdfium PROPERTY PDFIUM_RUNTIME_DLL "${_pdfium_runtime}")
elseif (APPLE)
    get_target_property(_pdfium_dylib PDFium::pdfium IMPORTED_LOCATION)
    set_property(TARGET thirdparty_pdfium PROPERTY PDFIUM_RUNTIME_DYLIB "${_pdfium_dylib}")
endif()

get_target_property(_pdfium_loc PDFium::pdfium IMPORTED_LOCATION)
get_target_property(_pdfium_implib PDFium::pdfium IMPORTED_IMPLIB)
message(STATUS "PDFium IMPORTED_LOCATION = ${_pdfium_loc}")
message(STATUS "PDFium IMPORTED_IMPLIB  = ${_pdfium_implib}")
