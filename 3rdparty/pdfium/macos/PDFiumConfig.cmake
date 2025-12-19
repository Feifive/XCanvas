include(FindPackageHandleStandardArgs)

set(PDFium_VERSION "145.0.7578.0")

find_path(PDFium_INCLUDE_DIR
    NAMES fpdfview.h
    PATHS ${CMAKE_CURRENT_LIST_DIR}/../include
)

find_library(PDFium_LIBRARY
    NAMES pdfium
    PATHS ${CMAKE_CURRENT_LIST_DIR}
    PATH_SUFFIXES lib
)

add_library(PDFium::pdfium SHARED IMPORTED)
set_target_properties(PDFium::pdfium PROPERTIES
    IMPORTED_LOCATION             "${PDFium_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PDFium_INCLUDE_DIR};${PDFium_INCLUDE_DIR}/cpp"
)

add_library(pdfium ALIAS PDFium::pdfium)

find_package_handle_standard_args(PDFium
    REQUIRED_VARS PDFium_LIBRARY PDFium_INCLUDE_DIR
    VERSION_VAR PDFium_VERSION
)
