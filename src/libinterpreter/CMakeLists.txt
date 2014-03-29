add_library(interpreter
  execution_visitor.cpp
  execution_context.cpp
)

target_link_libraries(interpreter parser)
