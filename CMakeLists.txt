project( casmi )
cmake_minimum_required( VERSION 2.6 )

enable_testing()

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/lib )

set( CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -Wextra -O3 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE" )

find_package( Boost 1.50 COMPONENTS program_options REQUIRED )

include_directories( src "${casmi_BINARY_DIR}/src" ${Boost_INCLUDE_DIR} )


# add dependency here, added include directories must be visible in subdirectories
add_subdirectory( "deps/gtest-1.7.0" )
include_directories( ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} )

add_subdirectory( "deps/hayai" )
include_directories( "deps/hayai/src" )

add_subdirectory( src )
add_subdirectory( tests )
add_subdirectory( bench )
