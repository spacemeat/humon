#!/usr/bin/python3

import sys
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


def buildObj (target, src, incDirs, debug, pic, cLanguage, tool):
    global objDir

    incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
    
    cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    if tool in ["gcc", "clang"]:
        gFlag = ""
        oFlag = "-O3"
        defs = "-D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L"
        fpicFlag = ""
        if debug:
            gFlag = "-g"
            oFlag = "-O0"
            defs = "-DDEBUG -D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L"
        if pic:
            fpicFlag = "-fPIC"
    
        if tool == "gcc":
            if cLanguage:
                cmplr = "gcc -std=c99"
            else:
                cmplr = "g++ -std=c++17"
        elif tool == "clang":
            if cLanguage:
                cmplr = "clang -std=c99"
            else:
                cmplr = "clang++ -std=c++17 -stdlib=libstdc++"
        
        cmd=f"{cmplr} -Wall -c {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {target} {src}"
        
    return doShellCommand(cmd)


def buildLib (target, srcList, incDirs, debug, tool):
    global objDir

    target=f"lib{target}{'-d' if debug else ''}.a"

    print (f"{dk_yellow_fg}Building static library {lt_yellow_fg}{target}{all_off}")
    
    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{objDir}/{os.path.basename(src)}{'-d' if debug else ''}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, debug, False, src.endswith(".c"), tool) != 0:
            canProceed = False

    cmd = 'echo "Failure to build target. Aborting."'
    if canProceed:
        cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
        if tool in ["gcc", "clang"]:
            cmd = f"ar cr -o {binDir}/{target} {' '.join(dotos)}"
    return doShellCommand(cmd)


def buildPythonLib (target ,srcList, incDirc, debug, tool):
    global objDir

    target=f"lib{target}{'-py'}.a"

    print (f"{dk_yellow_fg}Building static relocatable library {lt_yellow_fg}{target}{all_off}")
    
    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{objDir}/{os.path.basename(src)}{'-rd' if debug else '-r'}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, debug, True, src.endswith(".c"), tool) != 0:
            canProceed = False

    cmd = 'echo "Failure to build target. Aborting."'
    if canProceed:
        cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
        if tool in ["gcc", "clang"]:
            cmd = f"ar cr -o {binDir}/{target} {' '.join(dotos)}"
    return doShellCommand(cmd)


def buildSo (target, srcList, incDirs, debug, cLanguage, tool):
    global objDir
    global binDir

    if tool in ["gcc", "clang"]:
        soname=f"lib{target}{'-d' if debug else ''}.so"
        target = '.'.join([soname, "0.0.0"])
        soname = '.'.join([soname, "0"])

    print (f"{dk_yellow_fg}Building shared library {lt_yellow_fg}{target}{all_off}")

    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{objDir}/{os.path.basename(src)}{'-d' if debug else ''}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, debug, True, src.endswith(".c"), tool) != 0:
            canProceed = False

    defs = "-D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L"
    if debug:
        defs = "-DDEBUG -D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L"

    cmd = 'echo "Failure to build target. Aborting."'
    if canProceed:
        cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'

        if tool == "gcc":
            if cLanguage:
                cmplr = "gcc -std=c99"
            else:
                cmplr = "g++ -std=c++17"
        elif tool == "clang":
            if cLanguage:
                cmplr = "clang -std=c99"
            else:
                cmplr = "clang++ -std=c++17 -stdlib=libstdc++" 
        
        cmd = f"{cmplr} -Wall -shared -Wl,-soname,{soname} {defs} -o {binDir}/{target} {' '.join(dotos)}"

    return doShellCommand(cmd)


def buildExe (target, srcList, incDirs, libDirs, libs, debug, cLanguage, tool):
    global objDir
    global binDir

    print (f"{dk_yellow_fg}Building executable {lt_yellow_fg}{target}{all_off}")

    cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    if tool in ["gcc", "clang"]:
        gFlag = ""
        oFlag = "-O3"
        defs = "-D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L"
        fpicFlag = ""
        if debug:
            gFlag = "-g"
            oFlag = "-O0"
            defs = "-DDEBUG -D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200112L"
        
        incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
        libDirsArgs = [f"-L{libDir}" for libDir in libDirs]
        libsArgs    = [f"-l{lib}"    for lib in libs]
        
        if tool == "gcc":
            if cLanguage:
                cmplr = "gcc -std=c99"
            else:
                cmplr = "g++ -std=c++17"
        elif tool == "clang":
            if cLanguage:
                cmplr = "clang -std=c99"
            else:
                cmplr = "clang++ -std=c++17 -stdlib=libstdc++"

        cmd=f"{cmplr} -Wall {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {binDir}/{target} {' '.join(src)} {' '.join(libDirsArgs)} {' '.join(libsArgs)}"

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

    tool = "gcc"
    if 'clang' in sys.argv:
        tool = "clang"

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
    buildLib("humon", src, incDirs, True, tool)
    buildLib("humon", src, incDirs, False, tool)
    buildPythonLib("humon", src, incDirs, True, tool)
    buildSo("humon", src, incDirs, True, True, tool)
    buildSo("humon", src, incDirs, False, True, tool)

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
    buildExe("humon-test-d", src, ["include/humon"], [binDir], ["humon-d", "CppUTest", "CppUTestExt"], True, False, tool)
    buildExe("humon-test-r", src, ["include/humon"], [binDir], ["humon", "CppUTest", "CppUTestExt"], False, False, tool)

    src = ["apps/readmeSrc/usage.c"]
    buildExe("readmeSrc-c", src, ["include/humon"], [binDir], ["humon-d"], True, True, tool)

    src = ["apps/readmeSrc/usage.cpp"]
    buildExe("readmeSrc-cpp", src, ["include/humon"], [binDir], ["humon-d"], True, False, tool)

    src = ["apps/hux/hux.cpp"]
    buildExe("hux", src, ["include/humon"], [binDir], ["humon-d"], True, False, tool)
