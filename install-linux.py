#!/usr/bin/python3

import os
import shutil

include = 'include'
includeHumon = 'include/humon'
buildBin = 'build/bin'
usrLocalInclude = '/usr/local/include'
usrLocalIncludeHumon = '/usr/local/include/humon'
usrLocalLib = '/usr/local/lib'
usrLocalLibHumon = '/usr/local/lib/humon'

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
    
    if not os.path.exists(usrLocalLibHumon):
        os.mkdir(usrLocalLibHumon)
    
    shutil.copy('/'.join([buildBin, 'libhumon.a']), usrLocalLib)
    