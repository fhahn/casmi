add_library(interpreter
  execution_visitor.cpp
  execution_context.cpp
  value.cpp
)

target_link_libraries(interpreter parser)

add_executable(casmi main.cpp)
target_link_libraries(casmi interpreter ${Boost_LIBRARIES})
