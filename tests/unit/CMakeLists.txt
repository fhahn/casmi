add_executable(unittest_runner
    libsyntax/test_lexer_helpers.cpp
    libsyntax/test_parser.cpp
    libsyntax/test_ast.cpp
    libsyntax/test_type.cpp
    libinterpreter/test_execution_context.cpp
    libinterpreter/test_value.cpp
    libinterpreter/test_symbolic.cpp
)

target_link_libraries(unittest_runner gtest_main parser interpreter)
