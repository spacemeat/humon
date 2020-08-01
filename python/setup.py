# We're linking against '../build/bin/libhumon-d.a' which is built by `../build.py`.

from setuptools import setup, Extension
setup(name="humon",
      version="0.0.0",
      package_dir={ "": "src" },
      packages=["humon"],
      ext_package="humon",
      ext_modules=[Extension("_enumConsts", 
                             include_dirs = ['../include/humon', '../src'],
                             libraries = ['humon-py'],
                             library_dirs = ['../build/bin'],
                             extra_compile_args = ['-ggdb3', '-O0'],
                             sources = ["src/enumConstsModule/enumConstsModule.c"]),
                   Extension("humon",
                             include_dirs = ['../include/humon', '../src'],
                             libraries = ['humon-py'],
                             library_dirs = ['../build/bin'],
                             extra_compile_args = ['-ggdb3', '-O0'],
                             sources = ["src/humonModule/humonModule.c", 
                                        "src/humonModule/node-py.c",
                                        "src/humonModule/token-py.c",
                                        "src/humonModule/trove-py.c",
                                        "src/humonModule/utils.c"])
                   ])
      