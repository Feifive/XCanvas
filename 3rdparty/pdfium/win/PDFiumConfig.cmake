include(FindPackageHandleStandardArgs)

set(PDFium_VERSION "145.0.7578.0")

# ---------------- include ----------------
find_path(PDFium_INCLUDE_DIR
    NAMES fpdfview.h
    PATHS
        ${CMAKE_CURRENT_LIST_DIR}/../include
)

# ---------------- runtime dll ----------------
find_file(PDFium_RUNTIME
    NAMES pdfium.dll
    PATHS
        ${CMAKE_CURRENT_LIST_DIR}
    PATH_SUFFIXES bin
)

# ---------------- import lib ----------------
find_file(PDFium_IMPLIB
    NAMES pdfium.dll.lib
    PATHS
        ${CMAKE_CURRENT_LIST_DIR}
    PATH_SUFFIXES lib
)

# ---------------- imported target ----------------
add_library(PDFium::pdfium SHARED IMPORTED)
set_target_properties(PDFium::pdfium PROPERTIES
    IMPORTED_LOCATION             "${PDFium_RUNTIME}"
    IMPORTED_IMPLIB               "${PDFium_IMPLIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${PDFium_INCLUDE_DIR}"
)

add_library(pdfium ALIAS PDFium::pdfium)

find_package_handle_standard_args(PDFium
    REQUIRED_VARS PDFium_RUNTIME PDFium_IMPLIB PDFium_INCLUDE_DIR
    VERSION_VAR PDFium_VERSION
)
