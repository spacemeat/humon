#!/usr/bin/python3

import sys
import os
import os.path
import subprocess
from runpy import run_path

buildDir = "build"
objDir = '/'.join([buildDir, "obj"])
binDir = '/'.join([buildDir, "bin"])

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
    return subprocess.run(cmd, shell=True, check=True).returncode


if __name__ == "__main__":
    tests = [f for f in os.listdir('build/obj/bin')
               if os.path.isfile('build/obj/bin/' + f) and 
                  os.path.splitext(f)[0].startswith('test-')]
    numErrors = 0
    for test in tests:
        numErrors += doShellCommand('build/obj/bin/' + test)
    
    if numErrors == 0:
        print (f"\nTest binaries run: {len(tests)};  Errors returned: {numErrors}")
    