project ( casmi )
cmake_minimum_required( VERSION 2.6 )

enable_testing()

set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/bin)

set ( CMAKE_CXX_FLAGS "-std=c++11 -pedantic -O3 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE" )

include_directories( src )
include_directories( "${casmi_BINARY_DIR}/src" )

add_subdirectory( src )
add_subdirectory( tests )
