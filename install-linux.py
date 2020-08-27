#!/usr/bin/python3

import os
import shutil
import subprocess
from getVersion import getVersion

include = 'include'
includeHumon = 'include/humon'
buildBin = 'build/bin'
usrLocalInclude = '/usr/local/include'
usrLocalIncludeHumon = '/usr/local/include/humon'
usrLocalLib = '/usr/local/lib'
usrLocalLibHumon = '/usr/local/lib/humon'
usrLocalBin = '/usr/local/bin'

version = getVersion()
v3 = f"{version['major']}.{version['minor']}.{version['patch']}"
v2 = f"{version['major']}.{version['minor']}"
v1 = f"{version['major']}"


if __name__ == "__main__":
    if not os.path.exists(include):
        raise RuntimeError(f"Could not find include directory {include}")
    if not os.path.exists(buildBin):
        raise RuntimeError(f"Could not find bin directory {buildBin}")
    
    if not os.path.exists(usrLocalIncludeHumon):
        os.mkdir(usrLocalIncludeHumon)

    shutil.copy('/'.join([includeHumon, 'humon.h']), usrLocalIncludeHumon)
    shutil.copy('/'.join([includeHumon, 'humon.hpp']), usrLocalIncludeHumon)
    shutil.copy('/'.join([includeHumon, 'ansiColors.h']), usrLocalIncludeHumon)
    
    shutil.copy('/'.join([buildBin, 'libhumon.a']), usrLocalLib)

    soRealname = '/'.join([buildBin, f'libhumon.so.{v3}'])

    shutil.copy('/'.join([buildBin, f'libhumon.so.{v3}']), usrLocalLib)
    os.chmod('/'.join([usrLocalLib, f'libhumon.so.{v3}']), 0o755)
    ret = subprocess.run(f"ldconfig {usrLocalLib}", shell=True, check=True).returncode
    if ret != 0:
        raise RuntimeError("ldconfig failed")
    ret = subprocess.run(f"ln -sf libhumon.so.{v1} {'/'.join([usrLocalLib, f'libhumon.so'])}", shell=True, check=True).returncode
    if ret != 0:
        raise RuntimeError("ln failed")

    shutil.copy('/'.join([buildBin, 'hux']), usrLocalBin)
    