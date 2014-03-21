import sys
import subprocess
import os
import re

test_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        '../../build/bin/test_parser')



if __name__ == '__main__':
    check_file(sys.argv[1])
