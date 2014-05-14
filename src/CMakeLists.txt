set(SHARED_GLUE_HEADER "${CMAKE_BINARY_DIR}/src/shared_glue.h")


add_subdirectory( libsyntax )
add_subdirectory( libmiddle )
add_subdirectory( libutil )
add_subdirectory( libinterpreter )
