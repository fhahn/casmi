SET(LEXER "flex" CACHE STRING "flex parser")
SET(BISON "bison" CACHE STRING "yacc parser")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CUSTOM_COMMAND(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
                   COMMAND ${LEXER}
                   ARGS -olexer.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
                   TARGET ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
                   OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

ADD_CUSTOM_COMMAND(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/grammer.yy
                   COMMAND ${BISON}
                   ARGS --output parser.cpp
                        --defines=parser.tab.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/grammer.yy
                   TARGET ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
                   OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
)

ADD_CUSTOM_COMMAND(PRE_BUILD
                   SOURCE ${CMAKE_SOURCE_DIR}/src/libcasmrt/pp_casm_shared.h
                   COMMAND python 
                   ARGS ${CMAKE_SOURCE_DIR}/src/etc/shared_glue.py
                        ${CMAKE_SOURCE_DIR}/src/libcasmrt/pp_casm_shared.h
                        ${SHARED_GLUE_HEADER}
                   OUTPUT ${SHARED_GLUE_HEADER}
)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/parser.cpp GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp GENERATED)


add_library(parser 
  parser.cpp
  lexer.cpp
  driver.cpp
  lexer_helpers.cpp
  ast.cpp
  symbols.cpp
  types.cpp
  ast_dump_visitor.cpp
  ${SHARED_GLUE_HEADER}
)

target_link_libraries(parser y util)
