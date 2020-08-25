#!/usr/bin/python3

import sys
import os
import os.path
import subprocess
from runpy import run_path

all_off = '\033[0m'

dk_black_fg = '\033[30m'
dk_red_fg = '\033[31m'
dk_green_fg = '\033[32m'
dk_yellow_fg = '\033[33m'
dk_blue_fg = '\033[34m'
dk_magenta_fg = '\033[35m'
dk_cyan_fg = '\033[36m'
dk_white_fg = '\033[37m'

lt_black_fg = '\033[90m'
lt_red_fg = '\033[91m'
lt_green_fg = '\033[92m'
lt_yellow_fg = '\033[93m'
lt_blue_fg = '\033[94m'
lt_magenta_fg = '\033[95m'
lt_cyan_fg = '\033[96m'
lt_white_fg = '\033[97m'


def doShellCommand(cmd):
    print (f"{lt_black_fg}{cmd}{all_off}")
    return subprocess.run(cmd, shell=False, check=True).returncode


if __name__ == "__main__":
    # get linux tests
    tests = ['build/int/bin/' + f for f in os.listdir('build/int/bin')
               if os.path.isfile('build/int/bin/' + f) and 
                  os.path.splitext(f)[0].startswith('test-')]
    # get windows tests
    for d in os.listdir('build/int/bin'):
        dd = 'build/int/bin/' + d
        if os.path.isdir(dd) and d.startswith('test.'):
            for f in os.listdir(dd):
                if os.path.splitext(f)[1] == '.exe':
                    tests.append(dd + '/' + f)
                
    numErrors = 0
    for test in tests:
        numErrors += doShellCommand(test)
    
    if numErrors == 0:
        print (f"\nTest binaries run: {len(tests)};  Errors returned: {numErrors}")
    