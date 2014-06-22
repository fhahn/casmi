SET_SOURCE_FILES_PROPERTIES(${SHARED_GLUE_HEADER} GENERATED)

#add_library(interpreter
#  execution_visitor.cpp
#  execution_context.cpp
#  value.cpp
#  operators.cpp
#  builtins.cpp
#  symbolic.cpp
#  ${SHARED_GLUE_HEADER}
#)

#target_link_libraries(interpreter parser middle)

add_executable(casmi 
  ../libsyntax/parser.cpp
  ../libsyntax/lexer.cpp
  ../libsyntax/driver.cpp
  ../libsyntax/lexer_helpers.cpp
  ../libsyntax/ast.cpp
  ../libsyntax/symbols.cpp
  ../libsyntax/types.cpp
  ../libsyntax/ast_dump_visitor.cpp

  ../libmiddle/typecheck_visitor.cpp
  ../libmiddle/function_cycle_visitor.cpp

  ../libutil/exceptions.cpp

  execution_visitor.cpp
  execution_context.cpp
  value.cpp
  operators.cpp
  builtins.cpp
  symbolic.cpp
 
  main.cpp)
#target_link_libraries(casmi interpreter)
