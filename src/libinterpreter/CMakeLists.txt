SET_SOURCE_FILES_PROPERTIES(${SHARED_GLUE_HEADER} GENERATED)

add_library(interpreter
  execution_visitor.cpp
  execution_context.cpp
  value.cpp
  operators.cpp
  builtins.cpp
  symbolic.cpp
  ${SHARED_GLUE_HEADER}
)

target_link_libraries(interpreter parser middle)

add_executable(casmi 
  main.cpp
)
target_link_libraries(casmi interpreter)
