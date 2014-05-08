set(SHARED_GLUE_HEADER "${CMAKE_BINARY_DIR}/src/shared_glue.h")

add_custom_command(PRE_BUILD
                   SOURCE ${CMAKE_SOURCE_DIR}/src/librt/pp_casm_shared.h
                   COMMAND python 
                   ARGS ${CMAKE_SOURCE_DIR}/src/etc/shared_glue.py
                        ${CMAKE_SOURCE_DIR}/src/librt/pp_casm_shared.h
                        ${SHARED_GLUE_HEADER}
                   OUTPUT ${SHARED_GLUE_HEADER}
)


add_subdirectory( libsyntax )
add_subdirectory( libmiddle )
add_subdirectory( libutil )
add_subdirectory( libinterpreter )
