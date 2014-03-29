add_custom_target(
    check
    COMMAND python ${casmi_SOURCE_DIR}/src/etc/test_runner.py
    DEPENDS casmi
)
