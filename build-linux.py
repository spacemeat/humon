#!/usr/bin/python3

import sys
import os
import os.path
import subprocess
import ansi
from getVersion import getVersion

from runpy import run_path


buildDir = "build"
intDir = '/'.join([buildDir, "int"])
intBinDir = '/'.join([intDir, "bin"])
binDir = '/'.join([buildDir, "bin"])
docsDir = '/'.join([buildDir, "docs"])
docsDirC = '/'.join([docsDir, "humon-c"])
docsDirCpp = '/'.join([docsDir, "humon-cpp"])


def doShellCommand(cmd):
    print (f"{ansi.lt_black_fg}{cmd}{ansi.all_off}")
    return subprocess.run(cmd, shell=True, check=True).returncode


def buildObj(target, src, incDirs, flags, arch32bit, debug, pic, cLanguage, tool):
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

    intConfigDir = f"{intDir}/cfg{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"
    if not os.path.exists(intConfigDir):
        os.mkdir(intConfigDir)
    intConfigDir += f"/{finalTarget}"
    if not os.path.exists(intConfigDir):
        os.mkdir(intConfigDir)
    intBinConfigDir = f"{intBinDir}/cfg{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"
    if not os.path.exists(intBinConfigDir):
        os.mkdir(intBinConfigDir)

    print (f"{ansi.dk_yellow_fg}Building static library {ansi.lt_yellow_fg}{intBinConfigDir}/{finalTarget}{ansi.all_off}")

    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{intConfigDir}/{os.path.basename(src)}{'-32' if arch32bit else ''}{'-d' if debug else ''}.o"
        dotos.append(doto)
        if buildObj(doto, src, incDirs, flags, arch32bit, debug, False, cLanguage, tool) != 0:
            canProceed = False

    if canProceed:
        cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
        if tool in ["gcc", "clang"]:
            cmd = f"ar cr -o {intBinConfigDir}/{finalTarget} {' '.join(dotos)}"
            res = doShellCommand(cmd)
            if res != 0:
                return res
            
            cmd = f"cp {intBinConfigDir}/{finalTarget} {binDir}/{finalTarget}"
            return doShellCommand(cmd)

    cmd = 'echo "Failure to build target. Aborting."'
    return doShellCommand(cmd)


def buildPythonLib (target, srcList, incDirc, flags, arch32bit, debug, tool):
    global intDir

    target=f"lib{target}{'-py'}.a"

    print (f"{ansi.dk_yellow_fg}Building static relocatable library {ansi.lt_yellow_fg}{target}{ansi.all_off}")
    
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

    version = getVersion()
    v3 = f"{version['major']}.{version['minor']}.{version['patch']}"
    v2 = f"{version['major']}.{version['minor']}"
    v1 = f"{version['major']}"

    linkername = f"lib{target}.so"
    soname = linkername + f".{v1}"
    finalTarget = linkername + f".{v3}"
    #target=f"lib{target}{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}.so.{v3}"
    target = linkername

    intConfigDir = f"{intDir}/cfg{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"
    if not os.path.exists(intConfigDir):
        os.mkdir(intConfigDir)
    intConfigDir += f"/{linkername}"
    if not os.path.exists(intConfigDir):
        os.mkdir(intConfigDir)
    intBinConfigDir = f"{intBinDir}/cfg{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"
    if not os.path.exists(intBinConfigDir):
        os.mkdir(intBinConfigDir)
    
    print (f"{ansi.dk_yellow_fg}Building shared library {ansi.lt_yellow_fg}{intBinConfigDir}/{target}{ansi.all_off}")

    dotos = []
    canProceed = True
    for src in srcList:
        doto = f"{intConfigDir}/{os.path.basename(src)}{'-d' if debug else ''}.o"
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
        
        cmd = f"{cmplr} -Wall -Wextra -Werror -shared -Wl,-soname,{soname} {arch} {defs} -o {intBinConfigDir}/{target} {' '.join(dotos)}"
        res = doShellCommand(cmd)
        if res != 0:
            return res

        cmd = f"cp {intBinConfigDir}/{target} {binDir}/{finalTarget}"
        res = doShellCommand(cmd)
        if res != 0:
            return res
        
        cmd = f"ln -sf {finalTarget} {binDir}/{soname}"
        res = doShellCommand(cmd)
        if res != 0:
            return res
        
        cmd = f"ln -sf {soname} {binDir}/{linkername}"
        return doShellCommand(cmd)

    cmd = 'echo "Failure to build target. Aborting."'
    return doShellCommand(cmd)


def buildExe (target, srcList, incDirs, libDirs, staticLibs, sharedLibs, flags, arch32bit, debug, cLanguage, tool):
    global intDir
    global intBinDir
    global binDir

    finalTarget = target
    #target=f"{target}{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"

    intBinConfigDir = f"{intBinDir}/cfg{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"
    if not os.path.exists(intBinConfigDir):
        os.mkdir(intBinConfigDir)

    print (f"{ansi.dk_yellow_fg}Building executable {ansi.lt_yellow_fg}{intBinConfigDir}/{finalTarget}{ansi.all_off}")

    if tool in ["gcc", "clang"]:
        gFlag = "-g" if debug else ""
        oFlag = "-O0" if debug else "-O3"
        defs = f"{' -D_POSIX_C_SOURCE=200112L' if cLanguage else ''}"
        defs += f"{' -D_FILE_OFFSET_BITS' if cLanguage and not arch32bit else ''}"
        defs += flags
        arch = "-m32" if arch32bit else ""

        incDirsArgs = [f"-I{incDir}" for incDir in incDirs]
        libDirsArgs = [f"-L{libDir}" for libDir in libDirs]
        staticLibsArgs = [f"-l{lib}" for lib in staticLibs]
        sharedLibsArgs = [f"-l{lib}" for lib in sharedLibs]
        
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

        cmd = f'{cmplr} -Wall -Wextra -Werror {arch} {gFlag} {oFlag} {defs} {" ".join(incDirsArgs)} -o {intBinConfigDir}/{finalTarget} {" ".join(src)} {" ".join(libDirsArgs)} -Wl,-Bstatic {" ".join(staticLibsArgs)} -Wl,-Bdynamic {" ".join(sharedLibsArgs)} -Wl,-rpath,"\$ORIGIN"'
        res = doShellCommand(cmd)
        if res != 0:
            return res
        
        cmd = f"cp {intBinConfigDir}/{finalTarget} {binDir}/{finalTarget}"
        return doShellCommand(cmd)

    cmd = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    return doShellCommand(cmd)


def buildDocs():
    if not os.path.exists(docsDir):
        os.mkdir(docsDir)

    print (f"{ansi.dk_yellow_fg}Building API documents {ansi.lt_yellow_fg}{docsDir}{ansi.all_off}")

    try:
        isit = subprocess.run('whereis doxygen', shell=True, check=False, capture_output=True)
        output = str(isit.stdout)
        if isit.returncode == 0 and ':' in output and len(output.split(':')[1]) > 0:
            doShellCommand("doxygen dox-humon-c")
            doShellCommand("doxygen dox-humon-cpp")
        else:
            print ("doxygen not detected")
    except:
        print ("doxygen not detected")
   

if __name__ == "__main__":
    clean = False
    debug = False
    tool = ""
    arch32bit = False
    buildAll = False

    flags = ""

    # 32-bit build on 64-bit arch requires sudo apt install gcc-multilib g++-multilib

    for arg in sys.argv:
        if arg == "-clean":
            clean = True
        elif arg.startswith("-tool="):
            tool = arg.split('=')[1]
        elif arg == "-debug":
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

    if clean:
        cmd = 'rm -rf build'
        doShellCommand(cmd)
        quit()

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

                #targetName = f"humon{'-clang' if tool == 'clang' else '-gcc'}{'-32' if arch32bit else ''}{'-d' if debug else ''}"
                #intBinConfigDir = intBinDir + '/' + targetName
                
                buildLib("humon", src, incDirs, flags, arch32bit, debug, True, tool)
            #    buildPythonLib( "humon", src, incDirs, flags, arch32bit, True, tool)
                buildSo( "humon", src, incDirs, flags, arch32bit, debug, True, tool)

                print (f'{ansi.dk_yellow_fg}Generating test src in {ansi.lt_yellow_fg}./test{ansi.all_off}')
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

                buildExe("test", src, ["include"], [binDir], ["humon"], [], flags, arch32bit, debug, False, tool)

                src = ["apps/readmeSrc/usage.c"]
                buildExe("readmeSrc-c", src, ["include"], [binDir], ["humon"], [], flags, arch32bit, debug, True, tool)

                src = ["apps/readmeSrc/usage.cpp"]
                buildExe("readmeSrc-cpp", src, ["include"], [binDir], ["humon"], [], flags, arch32bit, debug, False, tool)

                src = ["apps/hux/hux.cpp"]
                buildExe("hux", src, ["include"], [binDir], [], ["humon"], flags, arch32bit, debug, False, tool)

    buildDocs()
