add_custom_target(
    bench-eval
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/bench.py
    DEPENDS casmi
)
