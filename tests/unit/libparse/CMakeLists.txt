add_executable(runUnitTests
    test_lexer_helpers.cpp
    test_parser.cpp
    test_ast.cpp
)

target_link_libraries(runUnitTests gtest_main parser)
