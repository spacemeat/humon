# We're linking against '../build/bin/libhumon-d.a' which is built by `../build.py`.

from setuptools import setup, Extension

setup(name="humon",
      version="0.0.0",
      packages=["humon"],
      ext_package="humon",
      ext_modules=[Extension("humon",
                             include_dirs = ['../include/humon', '../src'],
                             libraries = ['humon'],
                             library_dirs = ['../build/bin'],
                             extra_compile_args = ['-ggdb3', '-O0'],
                             sources = ["./humon/cpkg/enumConsts.c",
                                        "./humon/cpkg/humonModule.c", 
                                        "./humon/cpkg/node-py.c",
                                        "./humon/cpkg/token-py.c",
                                        "./humon/cpkg/trove-py.c",
                                        "./humon/cpkg/utils.c"])
                   ])
      