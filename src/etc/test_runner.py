import sys
import difflib
import subprocess
import os
import re

test_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../build/bin/casmi')
SYMBOLIC_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/symbolic/')
RUN_PASS_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/run-pass/')

RUN_FAIL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/run-fail/')

EXISTING_PARSE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../tests/integration/existing-tests/')



HR_LEN = 65
HR = '-' * 65

CMDLINE_RE = re.compile("// cmdline \"(.*)\"")

def get_options(filename):
    with open(filename, "rt") as f:
        lines = f.readlines()
        if len(lines) < 1:
            return []
        m = CMDLINE_RE.match(lines[0])
        if m: return m.groups()[0].split(" ")
    return []

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

def test_symbolic(filename):
    filename = filename.replace('./', '')
    short_filename = filename
    sys.stdout.write('\t[symbolic] '+short_filename)
    sys.stdout.flush()
    p1 = subprocess.Popen([test_exe,]+get_options(filename)+[filename,]+["-s"],
                          stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    (stdout, err) = p1.communicate()
    expected_path = filename.rsplit(".", 1)[0] + '.expected'
    if p1.returncode == 0:
        if os.path.exists(expected_path) and stdout.decode('utf8') != open(expected_path).read():
            expected_lines = open(expected_path).read().split("\n")
            got_lines = stdout.decode('utf8').split('\n')
            for l in difflib.unified_diff(expected_lines, got_lines):
                sys.stdout.write(l)
                sys.stdout.write('\n')

            sys.stdout.write(' ... fail (output did not match expected)\n\n')
            return (False, 'output did not match expected')
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


def test_run_pass(filename):
    short_filename = filename.replace(RUN_PASS_PATH, '')
    sys.stdout.write('\t[run-pass] '+short_filename)
    sys.stdout.flush()
    p1 = subprocess.Popen([test_exe,]+get_options(filename)+[filename,],
                          stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    (stdout, err) = p1.communicate()
    expected_path = filename.rsplit(".", 1)[0] + '.expected'
    if p1.returncode == 0:
        if os.path.exists(expected_path) and stdout.decode('utf8') != open(expected_path).read():
            print("")
            print("Expected:\n"+HR+"\n"+open(expected_path).read()+HR)
            print("\nGot:\n"+HR+"\n"+stdout.decode('utf8')+HR)

            sys.stdout.write(' ... fail (output did not match expected)\n')
            return (False, 'output did not match expected')
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

def run_existing_tests():
    return run_tests(EXISTING_PARSE_PATH, test_existing_parse)

def run_run_pass():
    return run_tests(RUN_PASS_PATH, test_run_pass)

def run_run_fail():
    return run_tests(RUN_FAIL_PATH, test_run_fail)

def run_symbolic():
    currdir = os.path.abspath(os.path.curdir)

    os.chdir(SYMBOLIC_PATH)

    ok_count, fail_count = run_tests("./", test_symbolic)
    os.chdir(currdir)
    return ok_count, fail_count


if __name__ == '__main__':
    if len(sys.argv) == 1:
        test_runners = [run_existing_tests, run_run_pass, run_run_fail, run_symbolic]
    elif len(sys.argv) == 2:
        if sys.argv[1] == "symbolic":
            test_runners = [run_symbolic]
        else:
            print("invalid arg")
            sys.exit(1)
    else:
        print("invalid arg")
        sys.exit(1)

    ok_count_sum = 0
    fail_count_sum = 0

    for r in test_runners:
        ok, fail = r()
        ok_count_sum += ok
        fail_count_sum += fail

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
