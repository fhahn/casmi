SET_SOURCE_FILES_PROPERTIES(${SHARED_GLUE_HEADER} GENERATED)

add_library(middle
  typecheck_visitor.cpp
  ${SHARED_GLUE_HEADER}
)
