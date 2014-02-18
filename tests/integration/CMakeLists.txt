add_executable(test_parser test_parser.c)
target_link_libraries(test_parser parser)

file(GLOB_RECURSE run-pass_files "run-pass/*.casm")

foreach(file ${run-pass_files})
    string(REPLACE "/" ";" dir_list ${file})
    list(REVERSE dir_list)
    list(GET dir_list 1 dir)
    list(GET dir_list 0 filename)
    add_test(NAME "${dir}/${filename}"
             COMMAND test_parser ${file}
    )
endforeach()
file(GLOB_RECURSE compile-fail_files "compile-fail/*.casm")

foreach(file ${compile-fail_files})
    string(REPLACE "/" ";" dir_list ${file})
    list(REVERSE dir_list)
    list(GET dir_list 1 dir)
    list(GET dir_list 0 filename)
    add_test(NAME "${dir}/${filename}"
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.py ${file}
    )
endforeach()


add_custom_target(
    check
    COMMAND ctest
    DEPENDS test_parser
)
