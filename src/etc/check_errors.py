import sys
import subprocess
import os
import re

test_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../build/bin/test_parser')


def check_file(filename):
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
    if p1.returncode == 0:
        print('\t[compile-fail] '+filename+' ... fail')
        sys.exit(1)
    else:
        error_re = re.compile("test2\.casm:(?P<line>\d+)\..+error: \\x1b\[0m\\x1b\[1m(?P<msg>.+)\\x1b\[0m$")
        for l in err.split("\n"):
            if "error: " in l:
                m = re.search(error_re, l)
                if int(m.groupdict()['line']) not in expected_errors:
                    print('  [compile-fail] '+filename+' ... fail')
                    print('    unexpected error message: '+l)
                    sys.exit(1)
                expected_error = expected_errors[int(m.groupdict()['line'])]
                if m.groupdict()['msg'].find(expected_error) == -1:
                    print('  [compile-fail] '+filename+' ... fail')
                    print('    expected error message not thrown; expected: '+expected_error)
                    print('    got error messages: '+m.groupdict()['msg'])
                    sys.exit(1)

        print('\t[compile-fail] '+filename+' ... ok')
        sys.exit(0)


if __name__ == '__main__':
    check_file(sys.argv[1])
