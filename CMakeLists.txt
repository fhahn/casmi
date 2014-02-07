project ( casmi )

set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/bin)

cmake_minimum_required( VERSION 2.6 )

set ( CMAKE_C_FLAGS "-std=c99 -pedantic -Wall -O3" )

include_directories( src )

add_subdirectory( src )
