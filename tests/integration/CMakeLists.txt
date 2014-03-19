add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser parser)


add_custom_target(
    check
    COMMAND python ${casmi_SOURCE_DIR}/src/etc/test_runner.py
    DEPENDS test_parser
)
