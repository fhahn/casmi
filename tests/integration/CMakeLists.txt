add_executable(test_parser test_parser.c)
target_link_libraries(test_parser parser)

file(GLOB_RECURSE files "*.casm")

foreach(file ${files})
    string(REPLACE "/" ";" dir_list ${file})
    list(REVERSE dir_list)
    list(GET dir_list 1 dir)
    list(GET dir_list 0 filename)
    add_test(NAME "${dir}/${filename}"
             COMMAND test_parser ${file}
    )
endforeach()

add_custom_target(
    check
    COMMAND ctest
    DEPENDS test_parser
)
