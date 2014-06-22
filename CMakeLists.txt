project( casmi )
cmake_minimum_required( VERSION 2.6 )

enable_testing()

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${casmi_BINARY_DIR}/lib )

# when -pendantic is used, -Wvariadic-macros cannot be disabled in gcc
set( CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -O3 -D_XOPEN_SOURCE=500 -D_BSD_SOURCE" )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCASMI_DEBUG")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_DEBUG} -pg")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

include_directories( src "${casmi_BINARY_DIR}/src")


# add dependency here, added include directories must be visible in subdirectories
add_subdirectory( "deps/gtest-1.7.0" )
include_directories( ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} )

add_subdirectory( "deps/hayai" )
include_directories( "deps/hayai/src" )

add_subdirectory( src )
add_subdirectory( tests )
add_subdirectory( bench )
