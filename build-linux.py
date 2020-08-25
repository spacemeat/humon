#!/usr/bin/python3

import sys
import os
import os.path
import subprocess
from runpy import run_path

buildDir = "build"
intDir = '/'.join([buildDir, "int"])
intBinDir = '/'.join([intDir, "bin"])
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


def buildObj (target, src, incDirs, flags, arch32bit, debug, pic, cLanguage, tool):
    global intDir

    incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
    
    cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    if tool in ["gcc", "clang"]:
        gFlag = "-g" if debug else ""
        oFlag = "-O0" if debug else "-O3"
        defs = f"{' -D_POSIX_C_SOURCE=200112L' if cLanguage else ''}"
        defs += f"{' -D_FILE_OFFSET_BITS' if cLanguage and not arch32bit else ''}"
        defs += flags
        arch = "-m32" if arch32bit else ""
        fpicFlag = "-fPIC" if pic else ""
    
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
        
        cmd=f"{cmplr} -Wall -Wextra -Werror -c {arch} {fpicFlag} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {target} {src}"
        
    return doShellCommand(cmd)


def buildLib (target, srcList, incDirs, flags, arch32bit, debug, cLanguage, tool):
    global intDir
    global intBinDir
    global binDir

    finalTarget = f"lib{target}.a"
    target=f"lib{target}{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}.a"

    print (f"{dk_yellow_fg}Building static library {lt_yellow_fg}{target}{all_off}")
    
    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{intDir}/{os.path.basename(src)}{'-32' if arch32bit else ''}{'-d' if debug else ''}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, flags, arch32bit, debug, False, cLanguage, tool) != 0:
            canProceed = False

    if canProceed:
        cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
        if tool in ["gcc", "clang"]:
            cmd = f"ar cr -o {intBinDir}/{target} {' '.join(dotos)}"
            res = doShellCommand(cmd)
            if res != 0:
                return res
            
            cmd = f"cp {intBinDir}/{target} {binDir}/{finalTarget}"
            return doShellCommand(cmd)

    cmd = 'echo "Failure to build target. Aborting."'
    return doShellCommand(cmd)


def buildPythonLib (target, srcList, incDirc, flags, arch32bit, debug, tool):
    global intDir

    target=f"lib{target}{'-py'}.a"

    print (f"{dk_yellow_fg}Building static relocatable library {lt_yellow_fg}{target}{all_off}")
    
    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{intDir}/{os.path.basename(src)}{'-rd' if debug else '-r'}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, flags, arch32bit, debug, True, True, tool) != 0:
            canProceed = False

    if canProceed:
        cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
        if tool in ["gcc", "clang"]:
            cmd = f"ar cr -o {intBinDir}/{target} {' '.join(dotos)}"
            return doShellCommand(cmd)

    cmd = 'echo "Failure to build target. Aborting."'
    return doShellCommand(cmd)


def buildSo (target, srcList, incDirs, flags, arch32bit, debug, cLanguage, tool):
    global intDir
    global intBinDir
    global binDir

    finalTarget = f"lib{target}.so.0.0.0"

    if tool in ["gcc", "clang"]:
        soname=f"lib{target}{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}.so"
        target = '.'.join([soname, "0.0.0"])
        soname = '.'.join([soname, "0.0"])

    print (f"{dk_yellow_fg}Building shared library {lt_yellow_fg}{target}{all_off}")

    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{intDir}/{os.path.basename(src)}{'-d' if debug else ''}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, flags, arch32bit, debug, True, cLanguage, tool) != 0:
            canProceed = False
    
    arch = "-m32" if arch32bit else ""
    defs = f"{' -D_POSIX_C_SOURCE=200112L' if cLanguage else ''}"
    defs += f"{' -D_FILE_OFFSET_BITS' if cLanguage and not arch32bit else ''}"
    defs += flags

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
        
        cmd = f"{cmplr} -Wall -Wextra -Werror -shared -Wl,-soname,{soname} {arch} {defs} -o {intBinDir}/{target} {' '.join(dotos)}"
        res = doShellCommand(cmd)
        if res != 0:
            return res

        cmd = f"cp {intBinDir}/{target} {binDir}/{finalTarget}"
        return doShellCommand(cmd)

    cmd = 'echo "Failure to build target. Aborting."'
    return doShellCommand(cmd)


def buildExe (target, srcList, incDirs, libDirs, libs, flags, arch32bit, debug, cLanguage, tool):
    global intDir
    global intBinDir
    global binDir

    finalTarget = target
    target=f"{target}{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"

    print (f"{dk_yellow_fg}Building executable {lt_yellow_fg}{target}{all_off}")

    if tool in ["gcc", "clang"]:
        gFlag = "-g" if debug else ""
        oFlag = "-O0" if debug else "-O3"
        defs = f"{' -D_POSIX_C_SOURCE=200112L' if cLanguage else ''}"
        defs += f"{' -D_FILE_OFFSET_BITS' if cLanguage and not arch32bit else ''}"
        defs += flags
        arch = "-m32" if arch32bit else ""

        incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
        libDirsArgs = [f"-L{intBinDir}" for libDir in libDirs]
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

        cmd = f"{cmplr} -Wall -Wextra -Werror {arch} {gFlag} {oFlag} {defs} {' '.join(incDirsArgs)} -o {intBinDir}/{target} {' '.join(src)} {' '.join(libDirsArgs)} {' '.join(libsArgs)}"
        res = doShellCommand(cmd)
        if res != 0:
            return res
        
        cmd = f"cp {intBinDir}/{target} {binDir}/{finalTarget}"
        return doShellCommand(cmd)

    cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    return doShellCommand(cmd)


if __name__ == "__main__":
    debug = False
    tool = ""
    arch32bit = False
    buildAll = False

    flags = ""

    # 32-bit build on 64-bit arch requires sudo apt install gcc-multilib g++-multilib

    for arg in sys.argv:
        if arg.startswith("-tool="):
            tool = arg.split('=')[1]
        elif arg.startswith("-debug"):
            debug = True
        elif arg == "-arch=":
            arch32bit = arg.split('=')[1] == '32'
        elif arg == "-buildAll":
            buildAll = True
        elif arg.startswith('-enumType='):
            flags += ' -DHUMON_ENUM_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-lineType='):
            flags += ' -DHUMON_LINE_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-colType='):
            flags += ' -DHUMON_COL_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-sizeType='):
            flags += ' -DHUMON_SIZE_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-swagBlock='):
            flags += ' -DHUMON_SWAG_BLOCKSIZE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-transcodeBlock='):
            flags += ' -DHUMON_TRANSCODE_BLOCKSIZE="' + arg.split('=')[1] + '"'
        elif arg == "-noChecks":
            flags += ' -DHUMON_NO_PARAMETER_CHECKS'
        elif arg == "-cavePerson":
            flags += ' -DHUMON_CAVEPERSON_DEBUGGING'
#        elif arg == "-noLineCol":
#            flags += ' -DHUMON_NO_LINE_COL'

    if debug:
        flags += ' -DDEBUG'
    
    if tool == '':
        tool = 'gcc'

    debugs = [debug]
    tools = [tool]
    arch32bits = [arch32bit]
    if buildAll:
        debugs = [True, False]
        tools = ['gcc', 'clang']
        arch32bits = [True, False]
    
    if not os.path.exists(buildDir):
        os.mkdir(buildDir)

    if not os.path.exists(intDir):
        os.mkdir(intDir)

    if not os.path.exists(intBinDir):
        os.mkdir(intBinDir)

    if not os.path.exists(binDir):
        os.mkdir(binDir)
    
    incDirs = [
        "include", 
        "src"
    ]

    for arch32bit in arch32bits:
        for tool in tools:
            for debug in debugs:
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
                
                buildLib("humon", src, incDirs, flags, arch32bit, debug, True, tool)
            #    buildPythonLib( "humon", src, incDirs, flags, arch32bit, True, tool)
                buildSo( "humon", src, incDirs, flags, arch32bit, debug, True, tool)

                libName = f"humon{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"

                print (f'{dk_yellow_fg}Generating test src in {lt_yellow_fg}./test{all_off}')
                doShellCommand("cd test && ./ztestMaker.py && cd ..")
                
                src = [
                    "test/ztest/ztest-main.cpp",
                    "test/ztest/ztest.cpp"
                ]

                tests = [f for f in os.listdir('test/ztest')
                        if os.path.isfile('test/ztest/' + f) and 
                            os.path.splitext(f)[0].startswith('gen-') and
                            os.path.splitext(f)[1] == ".cpp"]
                for test in tests:
                    src.append(''.join(["test/ztest/", test]))

                buildExe("test", src, ["include"], [binDir], [libName], flags, arch32bit, debug, False, tool)

                src = ["apps/readmeSrc/usage.c"]
                buildExe("readmeSrc-c", src, ["include"], [binDir], [libName], flags, arch32bit, debug, True, tool)

                src = ["apps/readmeSrc/usage.cpp"]
                buildExe("readmeSrc-cpp", src, ["include"], [binDir], [libName], flags, arch32bit, debug, False, tool)

                src = ["apps/hux/hux.cpp"]
                buildExe("hux", src, ["include"], [binDir], [libName], flags, arch32bit, debug, False, tool)
