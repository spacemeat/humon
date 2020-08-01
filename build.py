#!/usr/bin/python3

import os
import os.path
import subprocess

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


def buildObj (target, src, incDirs, debug, pic, cLanguage):
    global objDir

    gFlag = ""
    oFlag = "-O3"
    defs = ""
    fpicFlag = ""
    if debug:
        gFlag = "-ggdb3"
        oFlag = "-O0"
        defs = "-DDEBUG"
    if pic:
        fpicFlag = "-fPIC"
    
    incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
    
    if cLanguage:
        cmd=f"gcc -std=c99 -Wall -c {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {target} {src}"
    else:
        cmd=f"g++ -std=c++17 -Wall -c {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {target} {src}"
    
    return doShellCommand(cmd)


def buildLib (target, srcList, incDirs, debug):
    global objDir

    target=f"lib{target}{'-d' if debug else ''}.a"

    print (f"{dk_yellow_fg}Building static library {lt_yellow_fg}{target}{all_off}")
    
    dotos = []
    for src in srcList:
        doto = f"{objDir}/{os.path.basename(src)}{'-d' if debug else ''}.o"
        dotos.append(doto)
        buildObj(doto, src, incDirs, debug, False, src.endswith(".c"))

    cmd = f"ar cr -o {binDir}/{target} {' '.join(dotos)}"
    return doShellCommand(cmd)


def buildPythonLib (target ,srcList, incDirc, debug):
    global objDir

    target=f"lib{target}{'-py'}.a"

    print (f"{dk_yellow_fg}Building static relocatable library {lt_yellow_fg}{target}{all_off}")
    
    dotos = []
    for src in srcList:
        doto = f"{objDir}/{os.path.basename(src)}{'-rd' if debug else '-r'}.o"
        dotos.append(doto)
        buildObj(doto, src, incDirs, debug, True, src.endswith(".c"))

    cmd = f"ar cr -o {binDir}/{target} {' '.join(dotos)}"
    return doShellCommand(cmd)


def buildSo (target, srcList, incDirs, debug):
    global objDir
    global binDir

    soname=f"lib{target}{'-d' if debug else ''}.so"
    target = '.'.join([soname, "0.0.0"])
    soname = '.'.join([soname, "0"])

    print (f"{dk_yellow_fg}Building shared library {lt_yellow_fg}{target}{all_off}")

    dotos = []
    for src in srcList:
        doto = f"{objDir}/{os.path.basename(src)}{'-d' if debug else ''}.o"
        dotos.append(doto)
        buildObj(doto, src, incDirs, debug, True, src.endswith(".c"))

    cmd = f"g++ -std=c++17 -Wall -shared -Wl,-soname,{soname} -o {binDir}/{target} {' '.join(dotos)}"
    return doShellCommand(cmd)


def buildExe (target, srcList, incDirs, libDirs, libs, debug, cLanguage):
    global objDir
    global binDir

    print (f"{dk_yellow_fg}Building executable {lt_yellow_fg}{target}{all_off}")

    gFlag = ""
    oFlag = "-O3"
    defs = ""
    fpicFlag = ""
    if debug:
        gFlag = "-ggdb3"
        oFlag = "-O0"
        defs = "-DDEBUG"
    
    incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
    libDirsArgs = [f"-L{libDir}" for libDir in libDirs]
    libsArgs    = [f"-l{lib}"    for lib in libs]
    
    if cLanguage:
        cmd=f"gcc -std=c99 -Wall {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {binDir}/{target} {' '.join(src)} {' '.join(libDirsArgs)} {' '.join(libsArgs)}"
    else:
        cmd=f"g++ -std=c++17 -Wall {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {binDir}/{target} {' '.join(src)} {' '.join(libDirsArgs)} {' '.join(libsArgs)}"

    return doShellCommand(cmd)


if __name__ == "__main__":
    src = [
        "src/ansiColors.c",
        "src/encoding.c",
        "src/node.c",
        "src/parse.c",
        "src/printing.c",
        "src/tokenize.c",
        "src/trove.c",
        "src/utils.c",
        "src/vector.c"
    ]

    if not os.path.exists(buildDir):
        os.mkdir(buildDir)

    if not os.path.exists(objDir):
        os.mkdir(objDir)

    if not os.path.exists(binDir):
        os.mkdir(binDir)
    
    incDirs = [
        "include/humon", 
        "src"
    ]
    buildLib("humon", src, incDirs, True)
    buildLib("humon", src, incDirs, False)
    buildPythonLib("humon", src, incDirs, True)
    buildSo("humon", src, incDirs, True)
    buildSo("humon", src, incDirs, False)

    src = [
        "test/apiTests.cpp",
        "test/commentTests.cpp",
        "test/cppTests.cpp",
        "test/dataTests.cpp",
        "test/errorTests.cpp",
        "test/testMain.cpp",
        "test/utf8Tests.cpp",
        "test/vectorTests.cpp",
        "test/wakkaTests.cpp"
    ]
    buildExe("humon-test-d", src, ["include/humon"], [binDir], ["humon-d", "CppUTest", "CppUTestExt"], True, False)
    buildExe("humon-test-r", src, ["include/humon"], [binDir], ["humon-d", "CppUTest", "CppUTestExt"], False, False)

    src = ["apps/readmeSrc/usage.c"]
    buildExe("readmeSrc-c", src, ["include/humon"], [binDir], ["humon-d"], True, True)

    src = ["apps/readmeSrc/usage.cpp"]
    buildExe("readmeSrc-cpp", src, ["include/humon"], [binDir], ["humon-d"], True, False)

    src = ["apps/hux/hux.cpp"]
    buildExe("hux", src, ["include/humon"], [binDir], ["humon"], False, False)
