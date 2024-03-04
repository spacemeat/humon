#!/usr/bin/python3

import sys
import os
import os.path
import subprocess
#from typing import final
import ansi
from getVersion import getVersion

#from runpy import run_path


BUILD_DIR = "build"
INT_DIR = '/'.join([BUILD_DIR, "int"])
INT_BIN_DIR = '/'.join([INT_DIR, "bin"])
BIN_DIR = '/'.join([BUILD_DIR, "bin"])
DOCS_DIR = '/'.join([BUILD_DIR, "docs"])
DOCS_DIR_C = '/'.join([DOCS_DIR, "humon-c"])
DOCS_DIR_CPP = '/'.join([DOCS_DIR, "humon-cpp"])


def do_shell_command(cmd_str):
    '''
    Executes the command string in a shell subprocess. Since it is run in a subprocess,
    variable assignments or diretory changes will not be maintained in subsequent calls
    to do_shell_command.

    Parameters:
    - cmd_str: Full shell command string to execute.
    '''
    print (f"{ansi.lt_black_fg}{cmd_str}{ansi.all_off}")
    return subprocess.run(cmd_str, shell=True, check=True).returncode


def build_obj_file(target, src_file, inc_dirs, addl_flags,
                   is_arch_32_bit, is_debug, pic, is_lang_c, toolkit):
    '''
    Builds an object file from the given parameters. 

    Works by constructing a gcc or clang build command, and running it in a shell context.
    
    Parameters:
    - target: The file name of the resulting object file, minus the '.o' suffix.
    - src_file: A single source code path, relative to the top level directory.
    - inc_dirs: A list of include directories, relative to the top level directory.
    - addl_flags: A string containing additional flags to pass to the bulid command.
    - is_arch_32_bit: Whether to target a 32-bit or 64-bit architecture.
    - is_debug: Whether to include debug information in the build.
    - is_lang_c: Whether we are building C or C++ code.
    - toolkit: 'gcc' or 'clang'; if is_lang_c is false, the appropriate C++ variant will be used.
    '''
    inc_dirs_args = [f"-I{incDir}" for incDir in inc_dirs]

    cmd_str = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    if toolkit in ["gcc", "clang"]:
        g_flag = "-g" if is_debug else ""
        o_flag = "-O0" if is_debug else "-O3"
        defs = f"{' -D_POSIX_C_SOURCE=200112L' if is_lang_c else ''}"
        defs += f"{' -D_FILE_OFFSET_BITS' if is_lang_c and not is_arch_32_bit else ''}"
        arch = "-m32" if is_arch_32_bit else ""
        fpic_flag = "-fPIC" if pic else ""
        cmplr = ''

        if toolkit == "gcc":
            if is_lang_c:
                cmplr = "gcc -std=c99"
            else:
                cmplr = "g++ -std=c++17"
        elif toolkit == "clang":
            if is_lang_c:
                cmplr = "clang -std=c99"
            else:
                cmplr = "clang++ -std=c++17 -stdlib=libstdc++"

        cmd_str=(f"{cmplr} -Wall -Wextra -Werror -c {arch} {fpic_flag} {g_flag} {o_flag} "
                 f"{defs} {addl_flags} {' '.join(inc_dirs_args)} -o {target} {src_file}")

    return do_shell_command(cmd_str)


def build_lib_file(target, src_list, inc_dirs, addl_flags, is_arch_32_bit, is_debug,
                   is_lang_c, toolkit):
    '''
    Builds an executable static library file from the given parameters. 

    Works by building each source path into object files with build_obj_file(), and then archiving
    them by constructing a gcc or clang build command, and running it in a shell context.
    
    Parameters:
    - target: The file name of the resulting static library, minus the 'lib' prefix or '.a' suffix.
    - src_list: A list of source code paths, relative to the top level directory.
    - inc_dirs: A list of include directories, relative to the top level directory.
    - addl_flags: A string containing additional flags to pass to the bulid command.
    - is_arch_32_bit: Whether to target a 32-bit or 64-bit architecture.
    - is_debug: Whether to include debug information in the build.
    - is_lang_c: Whether we are building C or C++ code.
    - toolkit: 'gcc' or 'clang'; if is_lang_c is false, the appropriate C++ variant will be used.
    '''
    final_target = f"lib{target}.a"

    int_config_dir = (f"{INT_DIR}/cfg{'-clang' if toolkit == 'clang' else '-gcc'}"
                      f"{'-32' if is_arch_32_bit else ''}{'-d' if is_debug else ''}")
    if not os.path.exists(int_config_dir):
        os.mkdir(int_config_dir)
    int_config_dir += f"/{final_target}"
    if not os.path.exists(int_config_dir):
        os.mkdir(int_config_dir)
    int_bin_config_dir = (f"{INT_BIN_DIR}/cfg{'-clang' if toolkit == 'clang' else '-gcc'}"
                          f"{'-32' if is_arch_32_bit else ''}{'-d' if is_debug else ''}")
    if not os.path.exists(int_bin_config_dir):
        os.mkdir(int_bin_config_dir)

    print (f"{ansi.dk_yellow_fg}Building static library {ansi.lt_yellow_fg}{int_bin_config_dir}/"
           f"{final_target}{ansi.all_off}")

    dotos = []
    can_proceed = True
    for src_file in src_list:
        doto = (f"{int_config_dir}/{os.path.basename(src_file)}{'-32' if is_arch_32_bit else ''}"
                f"{'-d' if is_debug else ''}.o")
        dotos.append(doto)
        if build_obj_file(doto, src_file, inc_dirs, addl_flags, is_arch_32_bit, is_debug,
                          False, is_lang_c, toolkit) != 0:
            can_proceed = False

    if can_proceed:
        cmd_string = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
        if toolkit in ["gcc", "clang"]:
            cmd_string = f"ar cr -o {int_bin_config_dir}/{final_target} {' '.join(dotos)}"
            res = do_shell_command(cmd_string)
            if res != 0:
                return res

            cmd_string = f"cp {int_bin_config_dir}/{final_target} {BIN_DIR}/{final_target}"
            return do_shell_command(cmd_string)

    cmd_string = 'echo "Failure to build target. Aborting."'
    return do_shell_command(cmd_string)


def build_shared_object_file (target, src_list, inc_irs, addl_flags, is_arch_32_bit, is_debug,
                              is_lang_c, toolkit):
    '''
    Builds an executable shared object library file from the given parameters. 

    Works by building each source path into object files with build_obj_file(), and then linking
    them by constructing a gcc or clang build command, and running it in a shell context.
    
    Parameters:
    - target: The file name of the resulting shared object file, minus the 'lib' prefix and the
      '.so' suffix, as well as the version information in the name.
    - src_list: A list of source code paths, relative to the top level directory.
    - inc_dirs: A list of include directories, relative to the top level directory.
    - addl_flags: A string containing additional flags to pass to the bulid command.
    - is_arch_32_bit: Whether to target a 32-bit or 64-bit architecture.
    - is_debug: Whether to include debug information in the build.
    - is_lang_c: Whether we are building C or C++ code.
    - toolkit: 'gcc' or 'clang'; if is_lang_c is false, the appropriate C++ variant will be used.
    '''
    version = getVersion()
    v3 = f"{version['major']}.{version['minor']}.{version['patch']}"
    v1 = f"{version['major']}"

    linkername = f"lib{target}.so"
    soname = linkername + f".{v1}"
    final_target = linkername + f".{v3}"
    target = linkername

    int_config_dir = (f"{INT_DIR}/cfg{'-clang' if toolkit == 'clang' else '-gcc'}"
                      f"{'-32' if is_arch_32_bit else ''}{'-d' if is_debug else ''}")
    if not os.path.exists(int_config_dir):
        os.mkdir(int_config_dir)
    int_config_dir += f"/{linkername}"
    if not os.path.exists(int_config_dir):
        os.mkdir(int_config_dir)
    int_bin_config_dir = (f"{INT_BIN_DIR}/cfg{'-clang' if toolkit == 'clang' else '-gcc'}"
                          f"{'-32' if is_arch_32_bit else ''}{'-d' if is_debug else ''}")
    if not os.path.exists(int_bin_config_dir):
        os.mkdir(int_bin_config_dir)

    print (f"{ansi.dk_yellow_fg}Building shared library {ansi.lt_yellow_fg}{int_bin_config_dir}/"
           f"{target}{ansi.all_off}")

    dotos = []
    can_proceed = True
    for src_file in src_list:
        doto = f"{int_config_dir}/{os.path.basename(src_file)}{'-d' if is_debug else ''}.o"
        dotos.append(doto)
        if build_obj_file(doto, src_file, inc_irs, addl_flags, is_arch_32_bit, is_debug, True,
                          is_lang_c, toolkit) != 0:
            can_proceed = False

    arch = "-m32" if is_arch_32_bit else ""
    defs = f"{' -D_POSIX_C_SOURCE=200112L' if is_lang_c else ''}"
    defs += f"{' -D_FILE_OFFSET_BITS' if is_lang_c and not is_arch_32_bit else ''}"
    defs += addl_flags

    if can_proceed:
        cmd_string = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'

        cmplr = ''
        if toolkit == "gcc":
            if is_lang_c:
                cmplr = "gcc -std=c99"
            else:
                cmplr = "g++ -std=c++17"
        elif toolkit == "clang":
            if is_lang_c:
                cmplr = "clang -std=c99"
            else:
                cmplr = "clang++ -std=c++17 -stdlib=libstdc++"

        cmd_string = (f"{cmplr} -Wall -Wextra -Werror -shared -Wl,-soname,{soname} "
                      f"{arch} {defs} -o {int_bin_config_dir}/{target} {' '.join(dotos)}")
        res = do_shell_command(cmd_string)
        if res != 0:
            return res

        cmd_string = f"cp {int_bin_config_dir}/{target} {BIN_DIR}/{final_target}"
        res = do_shell_command(cmd_string)
        if res != 0:
            return res

        cmd_string = f"ln -sf {final_target} {BIN_DIR}/{soname}"
        res = do_shell_command(cmd_string)
        if res != 0:
            return res

        cmd_string = f"ln -sf {soname} {BIN_DIR}/{linkername}"
        return do_shell_command(cmd_string)

    cmd_string = 'echo "Failure to build target. Aborting."'
    return do_shell_command(cmd_string)


def build_exe_file (target, src_list, inc_dirs, lib_dirs, static_libs, shared_libs, addl_flags,
                    is_arch_32_bit, is_debug, is_lang_c, toolkit):
    '''
    Builds an executable file from the given parameters. 

    Works by constructing a gcc or clang build command, and running it in a shell context.
    
    Parameters:
    - target: The file name of the resulting executable file.
    - src_list: A list of source code paths, relative to the top level directory.
    - inc_dirs: A list of include directories, relative to the top level directory.
    - lib_dirs: A list of library directories, relative to the top level directory.
    - static_libs: A list of static libraries (.a files) to include in the build command.
    - shared_libs: A list of shared object libraries (.so files) to include in the build command.
    - addl_flags: A string containing additional flags to pass to the bulid command.
    - is_arch_32_bit: Whether to target a 32-bit or 64-bit architecture.
    - is_debug: Whether to include debug information in the build.
    - is_lang_c: Whether we are building C or C++ code.
    - toolkit: 'gcc' or 'clang'; if is_lang_c is false, the appropriate C++ variant will be used.
    '''
    final_target = target

    int_bin_config_dir = (f"{INT_BIN_DIR}/cfg{'-clang' if toolkit == 'clang' else '-gcc'}"
                          f"{'-32' if is_arch_32_bit else ''}{'-d' if is_debug else ''}")
    if not os.path.exists(int_bin_config_dir):
        os.mkdir(int_bin_config_dir)

    print (f"{ansi.dk_yellow_fg}Building executable {ansi.lt_yellow_fg}{int_bin_config_dir}/"
           f"{final_target}{ansi.all_off}")

    if toolkit in ["gcc", "clang"]:
        g_flag = "-g" if is_debug else ""
        o_flag = "-O0" if is_debug else "-O3"
        defs = f"{' -D_POSIX_C_SOURCE=200112L' if is_lang_c else ''}"
        defs += f"{' -D_FILE_OFFSET_BITS' if is_lang_c and not is_arch_32_bit else ''}"
        defs += addl_flags
        arch = "-m32" if is_arch_32_bit else ""

        inc_dirs_args = [f"-I{incDir}" for incDir in inc_dirs]
        lib_dirs_args = [f"-L{libDir}" for libDir in lib_dirs]
        static_libs_args = [f"-l{lib}" for lib in static_libs]
        shared_libs_args = [f"-l{lib}" for lib in shared_libs]

        cmplr = ''
        if toolkit == "gcc":
            if is_lang_c:
                cmplr = "gcc -std=c99"
            else:
                cmplr = "g++ -std=c++17"
        elif toolkit == "clang":
            if is_lang_c:
                cmplr = "clang -std=c99"
            else:
                cmplr = "clang++ -std=c++17 -stdlib=libstdc++"

        cmd_string = (f'{cmplr} -Wall -Wextra -Werror {arch} {g_flag} {o_flag} {defs} '
                      f'{" ".join(inc_dirs_args)} -o {int_bin_config_dir}/{final_target} '
                      f'{" ".join(src_list)} {" ".join(lib_dirs_args)} -Wl,-Bstatic '
                      f'{" ".join(static_libs_args)} -Wl,-Bdynamic {" ".join(shared_libs_args)} '
                      f'-Wl,-rpath,"$ORIGIN"')
        res = do_shell_command(cmd_string)
        if res != 0:
            return res

        cmd_string = f"cp {int_bin_config_dir}/{final_target} {BIN_DIR}/{final_target}"
        return do_shell_command(cmd_string)

    cmd_string = 'echo "No build tools set. Select from \"gcc\" or \"clang\"."'
    return do_shell_command(cmd_string)


def build_docs():
    ''' TODO: docstring '''
    if not os.path.exists(DOCS_DIR):
        os.mkdir(DOCS_DIR)

    version = getVersion()
    v3 = f"{version['major']}.{version['minor']}.{version['patch']}"

    print (f"{ansi.dk_yellow_fg}Building API documents {ansi.lt_yellow_fg}{DOCS_DIR}{ansi.all_off}")

    isit = subprocess.run('whereis doxygen', shell=True, check=False, capture_output=True)
    output = str(isit.stdout)
    if isit.returncode == 0 and ':' in output and len(output.split(':')[1]) > 0:
        do_shell_command(f'( cat dox-humon-c ; echo "PROJECT_NUMBER={v3}" ) | doxygen -')
        do_shell_command(f'( cat dox-humon-cpp ; echo "PROJECT_NUMBER={v3}" ) | doxygen -')
    else:
        print ("doxygen not detected")


def build_readme():
    ''' TODO: docstring '''
    print (f"{ansi.dk_yellow_fg}Building {ansi.lt_yellow_fg}README.md{ansi.all_off}")
    do_shell_command('./updateReadme.py')


def configure_from_args():
    ''' TODO: docstring '''
    is_clean = False
    is_debug = False
    toolkit = ""
    is_arch_32_bit = False
    build_all = False

    addl_flags = ""

    # 32-bit build on 64-bit arch requires sudo apt install gcc-multilib g++-multilib

    for arg in sys.argv:
        if arg == "-clean":
            is_clean = True
        elif arg.startswith("-tool="):
            toolkit = arg.split('=')[1]
        elif arg == "-debug":
            is_debug = True
        elif arg == "-arch=":
            is_arch_32_bit = arg.split('=')[1] == '32'
        elif arg == "-buildAll":
            build_all = True
        elif arg.startswith('-enumType='):
            addl_flags += ' -DHUMON_ENUM_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-lineType='):
            addl_flags += ' -DHUMON_LINE_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-colType='):
            addl_flags += ' -DHUMON_COL_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-sizeType='):
            addl_flags += ' -DHUMON_SIZE_TYPE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-swagBlock='):
            addl_flags += ' -DHUMON_SWAG_BLOCKSIZE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-transcodeBlock='):
            addl_flags += ' -DHUMON_TRANSCODE_BLOCKSIZE="' + arg.split('=')[1] + '"'
        elif arg.startswith('-addressBlock='):
            addl_flags += ' -DHUMON_ADDRESS_BLOCKSIZE="' + arg.split('=')[1] + '"'
        elif arg == "-noChecks":
            addl_flags += ' -DHUMON_NO_PARAMETER_CHECKS'
        elif arg == "-cavePerson":
            addl_flags += ' -DHUMON_CAVEPERSON_DEBUGGING'
#        elif arg == "-noLineCol":
#            flags += ' -DHUMON_NO_LINE_COL'

    return (is_clean, is_debug, toolkit, is_arch_32_bit, build_all, addl_flags)


def main():
    ''' TODO: docstring '''
    is_clean, is_debug, toolkit, is_arch_32_bit, build_all, addl_flags = configure_from_args()
    if is_clean:
        cmd = 'rm -r build/*'
        do_shell_command(cmd)
        sys.exit()

    if is_debug:
        addl_flags += ' -DDEBUG'

    if toolkit == '':
        toolkit = 'gcc'

    is_debug_list = [is_debug]
    toolkit_list = [toolkit]
    is_arch_32_bit_list = [is_arch_32_bit]
    if build_all:
        is_debug_list = [True, False]
        toolkit_list = ['gcc', 'clang']
        is_arch_32_bit_list = [True, False]

    if not os.path.exists(BUILD_DIR):
        os.mkdir(BUILD_DIR)

    if not os.path.exists(INT_DIR):
        os.mkdir(INT_DIR)

    if not os.path.exists(INT_BIN_DIR):
        os.mkdir(INT_BIN_DIR)

    if not os.path.exists(BIN_DIR):
        os.mkdir(BIN_DIR)

    inc_dirs = [
        "include",
        "src"
    ]

    for is_32_bit in is_arch_32_bit_list:
        for toolkit in toolkit_list:
            for is_debug in is_debug_list:
                src = [
                    "src/ansiColors.c",
                    "src/encoding.c",
                    "src/node.c",
                    "src/parse.c",
                    "src/printing.c",
                    "src/tokenize.c",
                    "src/trove.c",
                    "src/utils.c",
                    "src/vector.c",
                    "src/changes.c"
                ]

                build_lib_file("humon", src, inc_dirs, addl_flags, is_32_bit,
                               is_debug, True, toolkit)
                build_shared_object_file("humon", src, inc_dirs, addl_flags,
                                         is_32_bit, is_debug, True, toolkit)

                print (f'{ansi.dk_yellow_fg}Generating test src in {ansi.lt_yellow_fg}'
                       f'./test{ansi.all_off}')
                do_shell_command("cd test && ./ztestMaker.py; cd ..")

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

                build_exe_file("test", src, ["include"], [BIN_DIR], ["humon"], [],
                               addl_flags, is_arch_32_bit, is_debug, False, toolkit)

                src = ["apps/readmeSrc/usage.c"]
                build_exe_file("readmeSrc-c", src, ["include"], [BIN_DIR], ["humon"], [],
                               addl_flags, is_arch_32_bit, is_debug, True, toolkit)

                src = ["apps/readmeSrc/usage.cpp"]
                build_exe_file("readmeSrc-cpp", src, ["include"], [BIN_DIR], ["humon"], [],
                               addl_flags, is_arch_32_bit, is_debug, False, toolkit)

                src = ["apps/hux/hux.cpp"]
                build_exe_file("hux", src, ["include"], [BIN_DIR], [], ["humon"],
                               addl_flags, is_arch_32_bit, is_debug, False, toolkit)

    build_docs()
    build_readme()

if __name__ == "__main__":
    main()
