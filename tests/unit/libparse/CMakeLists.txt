add_executable(unittest_runner
    test_lexer_helpers.cpp
    test_parser.cpp
    test_ast.cpp
)

target_link_libraries(unittest_runner gtest_main parser)
