#!/usr/bin/python3

import sys
import os
import os.path
import subprocess

import ansi
from getVersion import getVersion

def doShellCommand(cmd):
    print (f"{ansi.lt_black_fg}{cmd}{ansi.all_off}")
    return subprocess.run(cmd, shell=False, check=True).returncode


if __name__ == "__main__":
    # get linux tests
    tests = []
    for d in os.listdir('build/int/bin'):
        bibc = 'build/int/bin/' + d
        if os.path.isfile(bibc + '/test'):
            tests.append(bibc + '/test')

    # get windows tests
    for d in os.listdir('build/int/bin'):
        bibc = 'build/int/bin/' + d
        if os.path.isdir(bibc) and d.startswith('test.'):
            for f in os.listdir(bibc):
                if os.path.splitext(f)[1] == '.exe':
                    tests.append(bibc + '/' + f)

    numErrors = 0
    for test in tests:
        numErrors += doShellCommand(test)
    
    if numErrors == 0:
        print (f"\nTest binaries run: {len(tests)};  Errors returned: {numErrors}")
    