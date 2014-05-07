add_library(interpreter
  execution_visitor.cpp
  execution_context.cpp
  value.cpp
  operators.cpp
)

target_link_libraries(interpreter parser middle)

add_executable(casmi main.cpp)
target_link_libraries(casmi interpreter profiler)
