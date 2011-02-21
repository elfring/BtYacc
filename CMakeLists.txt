cmake_minimum_required(VERSION 2.6.3)
project(BtYacc)
set(BtYacc_VERSION_MAJOR 3)
set(BtYacc_VERSION_MINOR 1)
set(BtYacc_VERSION_PATCH 1)

find_program(AWK NAMES awk gawk mawk PATHS ENV PATH)

if(NOT AWK)
   message(FATAL_ERROR "The tool \"awk\" was not found.")
endif()

set(BtYacc_SKELETON "${BtYacc_BINARY_DIR}/skeleton.c")
add_custom_command(OUTPUT ${BtYacc_SKELETON}
                   COMMAND "${AWK}" -f '${BtYacc_SOURCE_DIR}/skel2c' '${BtYacc_SOURCE_DIR}/btyaccpa.ske' > '${BtYacc_SKELETON}'
                   DEPENDS "${BtYacc_SOURCE_DIR}/btyaccpa.ske" "${BtYacc_SOURCE_DIR}/skel2c"
                   COMMENT "The skeleton source file will be generated.")

add_definitions(-DBTYACC_BUILD_USE_CONFIGURATION_HEADER)
configure_file("${BtYacc_SOURCE_DIR}/build.h.in" "${BtYacc_BINARY_DIR}/build.h")
include_directories("${BtYacc_SOURCE_DIR}" "${BtYacc_BINARY_DIR}")

set(BtYacc_SOURCES
    "${BtYacc_SKELETON}"
    "closure.c"
    "error.c"
    "lalr.c"
    "log.c"
    "lr0.c"
    "main.c"
    "mkpar.c"
    "output.c"
    "mstring.c"
    "reader.c"
    "readskel.c"
    "symtab.c"
    "verbose.c"
    "warshall.c"
    "write.c")

add_executable(btyacc ${BtYacc_SOURCES})

# Tobias Rautenkranz can help to generate some documentation.
# http://tobias.rautenkranz.ch/cmake/doxygen/
include(UseDoxygen OPTIONAL)

mark_as_advanced(AWK)
