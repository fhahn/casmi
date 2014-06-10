add_subdirectory( integration )
add_subdirectory( unit )

add_custom_target(
    check-unit
    COMMAND ${casmi_BINARY_DIR}/bin/unittest_runner
    DEPENDS unittest_runner
)

add_custom_target(
    check-integration
    COMMAND python ${casmi_SOURCE_DIR}/src/etc/test_runner.py
    DEPENDS casmi
)

add_custom_target(
    check
    DEPENDS check-unit check-integration
)
