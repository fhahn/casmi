SET(LEXER "lex" CACHE STRING "flex parser")
SET(BISON "bison" CACHE STRING "yacc parser")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CUSTOM_COMMAND(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
                   COMMAND ${LEXER}
                   ARGS -olexer.c
                   ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
                   TARGET ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
                   OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
)

ADD_CUSTOM_COMMAND(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/grammer.y
                   COMMAND ${BISON}
                   ARGS --output parser.c 
                        --defines=parser.tab.h
                        ${CMAKE_CURRENT_SOURCE_DIR}/grammer.y
                   TARGET ${CMAKE_CURRENT_BINARY_DIR}/parser.c
                   OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.c
)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/parser.c GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/lexer.c GENERATED)


add_library(parser parser.c lexer.c)
target_link_libraries(parser fl y)
