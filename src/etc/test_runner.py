import sys
import subprocess
import os
import re

test_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../build/bin/casmi')
RUN_PASS_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/run-pass/')

RUN_FAIL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/run-fail/')

EXISTING_PARSE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/existing-tests/')



HR_LEN = 65
HR = '-' * 65

def test_existing_parse(filename):
    p1 = subprocess.Popen([test_exe, '--parse-only', filename], stderr=subprocess.PIPE)
    err = p1.communicate()[1]

    short_filename = filename.replace(EXISTING_PARSE_PATH, '')
    if p1.returncode == 0:
        print('\t[existing-parse] '+short_filename+' ... ok')
        return (True, '')
    else:
        print('\t[existing-parse] '+short_filename+' ... fail')
        error = ("{}\n"
                 "failed test {}\n"
                 "output: {}\n"
                 "{}\n"
                )
        return (False, error.format(HR,
                                    filename,
                                    err.decode(encoding='UTF-8'),
                                    HR))


def test_run_pass(filename):
    short_filename = filename.replace(RUN_PASS_PATH, '')
    sys.stdout.write('\t[run-pass] '+short_filename)
    sys.stdout.flush()

    p1 = subprocess.Popen([test_exe, filename], stderr=subprocess.PIPE)
    err = p1.communicate()[1]
    if p1.returncode == 0:
        sys.stdout.write(' ... ok\n')
        return (True, '')
    else:
        sys.stdout.write(' ... fail\n')
        error = ("{}\n"
                 "failed test {}\n"
                 "output: {}\n"
                 "{}\n"
                )
        return (False, error.format(HR,
                                    filename,
                                    err.decode(encoding='UTF-8'),
                                    HR))

RUN_FAIL_TEMPLATE = ("{}\n"
                     "failed test {}\n"
                     "output: {}\n"
                     "{}\n"
)

def test_run_fail(filename):
    short_filename = filename.replace(RUN_FAIL_PATH, '')
    sys.stdout.write('\t[run-fail] '+short_filename )
    sys.stdout.flush()

    expected_errors = {}

    with open(filename, "rt") as f:
        line_number = 1
        for l in f.readlines():
            parts = l.split("//~")
            if len(parts) == 2:
                line_offset = 0
                for i in range(0, len(parts[1])):
                    if parts[1][i] != '^':
                        break
                    line_offset += 1
                errors =  expected_errors.get(line_number-line_offset, [])
                errors.append(parts[1][line_offset:].strip())
                expected_errors[line_number-line_offset] = errors
            line_number += 1

    p1 = subprocess.Popen([test_exe, filename], stderr=subprocess.PIPE)
    err = p1.communicate()[1]
    err = err.decode(encoding='UTF-8')

    if p1.returncode == 0:
        sys.stdout.write('... fail\n')
        return (False, RUN_FAIL_TEMPLATE.format(HR,
                                                    filename,
                                                    'Test did run, but should fail',
                                                    HR
            ))
    elif p1.returncode == 1:
        error_re = re.compile(filename+":(?P<line>\d+)\..+error: \\x1b\[0m\\x1b\[1m(?P<msg>.+)\\x1b\[0m$")
        for l in err.split("\n"):
            if "error: " in l:
                m = re.search(error_re, l)
                if int(m.groupdict()['line']) not in expected_errors:
                    sys.stdout.write('... fail\n')
                    return (False, RUN_FAIL_TEMPLATE.format(HR,
                                    filename,
                                    'unexpected error message: '+l,
                                    HR))
                found = False
                for expected_error in expected_errors[int(m.groupdict()['line'])]:
                    if m.groupdict()['msg'].find(expected_error) != -1:
                        found = True

                if not found:
                    sys.stdout.write('... fail\n')
                    return (False, RUN_FAIL_TEMPLATE.format(HR,
                                                filename,
                                                ('expected error message not thrown; expected: '+expected_error,
                                                '\ngot error messages: '+m.groupdict()['msg']),
                                                HR))

        sys.stdout.write('... ok\n')
        return (True, '')
    else:
        sys.stdout.write('... fail\n')
        return (False, RUN_FAIL_TEMPLATE.format(HR,
                                                    filename,
                                                    'Test did fail with error code != 1',
                                                    HR
            ))


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

    ok_count, fail_count = run_tests(EXISTING_PARSE_PATH, test_existing_parse)
    ok_count_sum += ok_count
    fail_count_sum += fail_count
 
    ok_count, fail_count = run_tests(RUN_PASS_PATH, test_run_pass)
    ok_count_sum += ok_count
    fail_count_sum += fail_count

    ok_count, fail_count = run_tests(RUN_FAIL_PATH, test_run_fail)
    ok_count_sum += ok_count
    fail_count_sum += fail_count
    
    print('')
    print(HR)
    if fail_count_sum == 0:
        print('\n\nTest summary (OK):')
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
