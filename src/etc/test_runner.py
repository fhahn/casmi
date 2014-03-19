import sys
import subprocess
import os
import re

test_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../build/bin/test_parser')
RUN_PASS_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/run-pass/')

COMPILE_FAIL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/compile-fail/')


HR_LEN = 65
HR = '-' * 65

def test_run_pass(filename):
    p1 = subprocess.Popen([test_exe, filename], stderr=subprocess.PIPE)
    err = p1.communicate()[1]

    short_filename = filename.replace(RUN_PASS_PATH, '')
    if p1.returncode == 0:
        print('\t[run-pass] '+short_filename+' ... ok')
        return (True, '')
    else:
        print('\t[run-pass] '+short_filename+' ... fail')
        error = ("{}\n"
                 "failed test {}\n"
                 "output: {}\n"
                 "{}\n"
                )
        return (False, error.format(HR,
                                    filename,
                                    err.decode(encoding='UTF-8'),
                                    HR))

COMPILE_FAIL_TEMPLATE = ("{}\n"
                         "failed test {}\n"
                         "output: {}\n"
                         "{}\n"
)

def test_compile_fail(filename):

    expected_errors = {}

    with open(filename, "rt") as f:
        line_number = 1
        for l in f.readlines():
            parts = l.split("//~")
            if len(parts) == 2:
                expected_errors[line_number] = parts[1].strip()
            line_number += 1

    p1 = subprocess.Popen([test_exe, filename], stderr=subprocess.PIPE)
    err = p1.communicate()[1]
    err = err.decode(encoding='UTF-8')
    short_filename = filename.replace(COMPILE_FAIL_PATH, '')
    if p1.returncode == 0:
        print('\t[compile-fail] '+short_filename+' ... fail')
        return (False, COMPILE_FAIL_TEMPLATE.format(HR,
                                                    filename,
                                                    'Test did compile, but should fail',
                                                    HR
            ))
    else:
        error_re = re.compile(filename+":(?P<line>\d+)\..+error: \\x1b\[0m\\x1b\[1m(?P<msg>.+)\\x1b\[0m$")
        for l in err.split("\n"):
            if "error: " in l:
                m = re.search(error_re, l)
                if int(m.groupdict()['line']) not in expected_errors:
                    print('  [compile-fail] '+short_filename+' ... fail')
                    return (False, COMPILE_FAIL_TEMPLATE.format(HR,
                                    filename,
                                    'unexpected error message: '+l,
                                    HR))
                expected_error = expected_errors[int(m.groupdict()['line'])]
                if m.groupdict()['msg'].find(expected_error) == -1:
                    print('  [compile-fail] '+short_filename+' ... fail')
                    return (False, COMPILE_FAIL_TEMPLATE.format(HR,
                                                filename,
                                                ('expected error message not thrown; expected: '+expected_error,
                                                '\ngot error messages: '+m.groupdict()['msg']),
                                                HR))

        print('\t[compile-fail] '+short_filename+' ... ok')
        return (True, '')



def run_tests(directory, test_fn, verbose=False):
    results = []
    for dirname, dirnames, filenames in os.walk(directory):
        results += [test_fn(os.path.join(dirname, test_file)) for test_file in filenames if test_file.endswith(".casm")]

    ok_results = [ok for (ok,_) in results if ok]
    fail_results = [msg for (ok, msg) in results if not ok]

    print('run {} tests, ok: {}\tfailed: {}'.format(
        len(ok_results)+len(fail_results),
        len(ok_results),
        len(fail_results)
    ))

    if verbose:
        print('\n'.join(fail_results))

    return (len(ok_results), len(fail_results))


if __name__ == '__main__':
    ok_count_sum = 0
    fail_count_sum = 0

    ok_count, fail_count = run_tests(RUN_PASS_PATH, test_run_pass)
    ok_count_sum += ok_count
    fail_count_sum += fail_count

    ok_count, fail_count = run_tests(COMPILE_FAIL_PATH, test_compile_fail)
    ok_count_sum += ok_count
    fail_count_sum += fail_count
    
    print('')
    print(HR)
    if fail_count_sum == 0:
        print('\nTest summary (OK):')
    else:
        print('\nTest summary (FAIL):')
    print('  run {} tests, ok: {}\tfailed: {}\n'.format(
        ok_count_sum+fail_count_sum,
        ok_count_sum,
        fail_count_sum
    ))
    print(HR)

    if fail_count_sum == 0:
        sys.exit(0)
    else:
        sys.exit(1)