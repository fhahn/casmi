add_executable(bench_method_dispatch
  method_dispatch.cpp
  pointer_vs_move.cpp
)

target_link_libraries(bench_method_dispatch
  hayai_main
)
