add_library(interpreter
  execution_visitor.cpp
)

target_link_libraries(interpreter parser)
