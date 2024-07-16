''' Pyke makefile for humon.'''
from os import listdir
from pathlib import Path
import pyke as p

class ZTestMaker(p.CommandPhase):
    ''' Command to build ztests.'''
    def __init__(self, options: dict | None = None, dependencies = None):
        super().__init__(None, dependencies)
        self.options |= {
            'name': 'gen_test',
            'run_dir': '{project_anchor}/test',
            'posix_command': './ztestMaker.py'
        }
        self.options |= (options or {})

    def compute_file_operations(self):
        ''' Generate file operations.'''
        proj_path = Path(self.opt_str('project_anchor'))
        ztest_maker_path = proj_path / 'test/ztestMaker.py'
        input_files = [
            p.FileData(ztest_maker_path, 'script', None),
        ]
        output_files = [
            p.FileData(Path(proj_path / 'test/ztest/gen-test-runners.h'), 'header', self),
            p.FileData(Path(proj_path / 'test/ztest/gen-test-runners.cpp'), 'source', self),
        ]
        for f in listdir(proj_path / 'test'):
            if f.endswith('.hpp'):
                g = f'gen-{f[:-3]}cpp'
                input_files.append(p.FileData(proj_path / 'test' / f, 'generator source', None))
                output_files.append(p.FileData(proj_path / 'test/ztest' / g, 'source', self))

        self.record_file_operation(
            input_files,
            output_files,
            'generate')

class UpdateReadme(p.CommandPhase):
    ''' Command to build ztests.'''
    def __init__(self, options: dict | None = None, dependencies = None):
        super().__init__(None, dependencies)
        self.options |= {
            'name': 'gen_test',
            'posix_command': './updateReadme.py'
        }
        self.options |= (options or {})

    def compute_file_operations(self):
        ''' Generate file operations.'''
        proj_path = Path(self.opt_str('project_anchor'))
        update_readme_path = proj_path / 'updateReadme.py'
        readme_src_path = Path(self.opt_str('project_anchor')) / 'apps/readmeSrc'
        input_files = [
            p.FileData(update_readme_path, 'script', None),
            p.FileData(proj_path / 'README-preprocess.md', 'script', None),
            p.FileData(readme_src_path / 'hudo.hu', 'script', None),
            p.FileData(readme_src_path / 'materials.hu', 'script', None),
            p.FileData(readme_src_path / 'usage.c', 'script', None),
            p.FileData(readme_src_path / 'usage.cpp', 'script', None),
        ]
        output_files = [
            p.FileData(proj_path / 'README.md', 'script', None),
        ]
        for f in listdir(proj_path / 'test'):
            if f.endswith('.hpp'):
                g = f'gen-{f[:-3]}cpp'
                input_files.append(p.FileData(proj_path / 'test' / f, 'generator source', None))
                output_files.append(p.FileData(proj_path / 'test/ztest' / g, 'source', self))

        self.record_file_operation(
            input_files,
            output_files,
            'generate')

class MakeDocsForC(p.CommandPhase):
    ''' Command to build ztests.'''
    def __init__(self, options: dict | None = None, dependencies = None):
        super().__init__(None, dependencies)
        self.options |= {
            'name': 'make_docs_for_c',
            'posix_command': '( cat dox-humon-c ; echo "PROJECT_NUMBER={version}" ) | doxygen -',
        }
        self.options |= (options or {})

    def compute_file_operations(self):
        ''' Generate file operations.'''
        proj_path = Path(self.opt_str('project_anchor'))
        dox_path = proj_path / 'dox-humon-c'
        inc_path = proj_path / 'include'
        out_dir = Path(self.opt_str('{build_anchor}')) / 'docs'
        input_files = [
            p.FileData(dox_path, 'script', None),
            p.FileData(inc_path / 'humon/humon.h', 'header', None),
            p.FileData(inc_path / 'humon/version.h', 'header', None),
            p.FileData(inc_path / 'humon/ansiColors.h', 'header', None),
        ]
        output_files = [
            p.FileData(out_dir, 'dir', self),
        ]

        self.record_file_operation(
            input_files,
            output_files,
            'generate')

class MakeDocsForCpp(p.CommandPhase):
    ''' Command to build ztests.'''
    def __init__(self, options: dict | None = None, dependencies = None):
        super().__init__(None, dependencies)
        self.options |= {
            'name': 'make_docs_for_cpp',
            'posix_command': '( cat dox-humon-cpp ; echo "PROJECT_NUMBER={version}" ) | doxygen -',
        }
        self.options |= (options or {})

    def compute_file_operations(self):
        ''' Generate file operations.'''
        proj_path = Path(self.opt_str('project_anchor'))
        dox_path = proj_path / 'dox-humon-cpp'
        inc_path = proj_path / 'include'
        out_dir = Path(self.opt_str('{build_anchor}')) / 'docs'
        input_files = [
            p.FileData(dox_path, 'script', None),
            p.FileData(inc_path / 'humon/humon.hpp', 'header', None),
        ]
        output_files = [
            p.FileData(out_dir, 'dir', self),
        ]

        self.record_file_operation(
            input_files,
            output_files,
            'generate')

TEST = 'test'
STATIC_LIB = 'static_lib'
SHARED_LIB = 'shared_lib'
README = 'readme'
DOCS = 'docs'

# -- static lib

compile_humon_lib_static = p.CompilePhase({
    'name': 'humon_compile',
    'group': STATIC_LIB,
    'language': 'c',
    'language_version': '11',
    'src_dir': 'src',
    'sources': [
        'ansiColors.c',
        'changes.c',
        'encoding.c',
        'node.c',
        'parse.c',
        'printing.c',
        'token.c',
        'tokenize.c',
        'trove.c',
        'utils.c',
        'vector.c'
    ],
    'definitions': ['_POSIX_C_SOURCE=200112L',
                    '_FILE_OFFSET_BITS']
})

link_static_lib = p.ArchivePhase({
    'name': 'humon_archive',
    'group': STATIC_LIB,
    'archive_basename': 'humon'
}, compile_humon_lib_static)

# -- shared lib

compile_humon_lib_shared = compile_humon_lib_static.clone({
    'group': SHARED_LIB,
})

link_shared_lib = p.LinkToSharedObjectPhase({
    'name': 'humon_link_to_so',
    'group': SHARED_LIB,
    'shared_object_basename': 'humon'
}, compile_humon_lib_shared)

# -- test

generate_test = ZTestMaker({'group': TEST})

compile_test = p.CompilePhase({
    'name': 'compile',
    'group': TEST,
    'language': 'c++',
    'language_version': '17',
    'src_dir': 'test',
    'obj_dir': 'int/test',
    'sources': ['ztest/ztest.cpp', 'ztest/ztest-main.cpp'],
}, generate_test)

link_test_static = p.LinkToExePhase({
    'name': 'link_to_exe_static',
    'group': TEST,
    'exe_basename': 'humon_test_static',
}, [link_static_lib, compile_test])

link_test_shared = p.LinkToExePhase({
    'name': 'link_to_exe_shared',
    'group': TEST,
    'exe_basename': 'humon_test_shared',
}, [link_shared_lib, compile_test])

# -- hux

compile_hux_static = p.CompilePhase({
    'name': 'hux_compile',
    'group': STATIC_LIB,
    'language': 'c++',
    'language_version': '17',
    'src_dir': 'apps/hux',
    'sources': ['hux.cpp'],
})

link_hux_static = p.LinkToExePhase({
    'name': 'hux_link_to_exe_static',
    'group': STATIC_LIB,
    'exe_basename': 'hux',
    'exe_dir': 'bin/static'
}, [link_static_lib, compile_hux_static])

compile_hux_shared = compile_hux_static.clone({
    'group': SHARED_LIB,
})

link_hux_shared = p.LinkToExePhase({
    'name': 'hux_link_to_exe_shared',
    'group': SHARED_LIB,
    'exe_basename': 'hux',
    'exe_dir': 'bin/shared'
}, [link_shared_lib, compile_hux_static])

# -- readme

build_readme_c = p.CompileAndLinkToExePhase({
    'name': 'readmeSrc-c_compile_link',
    'group': README,
    'language': 'c',
    'language_version': '11',
    'src_dir': 'apps/readmeSrc',
    'sources': ['usage.c'],
    'obj_dir': 'usage-c'
}, link_shared_lib)

build_readme_cpp = p.CompileAndLinkToExePhase({
    'name': 'readmeSrc-cpp_compile_link',
    'group': README,
    'language': 'c++',
    'language_version': '17',
    'src_dir': 'apps/readmeSrc',
    'sources': ['usage.cpp'],
    'obj_dir': 'usage-cpp',
    'action_map': {'build': ['build', 'run']},
}, link_shared_lib)

update_readme = UpdateReadme({
    'name': 'update_readme',
    'group': README,
}, [build_readme_c, build_readme_cpp])

# -- docs

whereis_doxygen = p.CommandPhase({
    'name': 'find_doxygen',
    'group': DOCS,
    'posix_command': 'whereis doxygen'
})

make_doc_dir = p.CommandPhase({
    'name': 'make_doc_dir',
    'group': DOCS,
    'posix_command': 'mkdir -p {build_anchor}/docs'
}, whereis_doxygen)

docs_for_c = MakeDocsForC({'group': DOCS}, make_doc_dir)
docs_for_cpp = MakeDocsForCpp({'group': DOCS}, make_doc_dir)

p.get_main_phase().depend_on([link_test_static, link_test_shared, update_readme,
                              link_hux_static, link_hux_shared, docs_for_c, docs_for_cpp])
