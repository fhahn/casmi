import sys
import subprocess
import os

test_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../build/bin/test_parser')


def check_file(filename):
    ret = subprocess.call([test_exe, filename])
    if ret == 0:
        print('\t[compile-fail] '+filename+' ... fail')
        sys.exit(1)
    else:
        print('\t[compile-fail] '+filename+' ... ok')
        sys.exit(0)


if __name__ == '__main__':
    check_file(sys.argv[1])
