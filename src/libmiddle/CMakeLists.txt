SET_SOURCE_FILES_PROPERTIES(${SHARED_GLUE_HEADER} GENERATED)

add_library(middle
  typecheck_visitor.cpp
  function_cycle_visitor.cpp
  ${SHARED_GLUE_HEADER}
)

# this should force parser to be compiled before middle, but there should
# be a better way to do this
target_link_libraries(middle parser)
