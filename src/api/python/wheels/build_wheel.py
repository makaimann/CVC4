###############################################################################
# Top contributors (to current version):
#   Makai Mann
#
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2021 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# ############################################################################
#
# Script for building wheels distribution for pycvc5
#
##

import os
import re
import sys
import platform
import subprocess
import multiprocessing
import shutil
import glob

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion

def get_root_path():
    # expecting this script to be in src/api/python/wheels
    # root is 4 directories up (5 dirnames because start with file)
    root_path = __file__
    for i in range(5):
        root_path = os.path.dirname(root_path)
    return os.path.abspath(root_path)


def get_cvc5_version():
    root_path = get_root_path()

    # read CMakeLists.txt to get version number
    version = dict()
    str_pattern = 'set\(CVC5_(?P<component>MAJOR|MINOR|RELEASE)\s*(?P<version>\d+)\)'
    pattern = re.compile(str_pattern)
    with open(root_path + '/CMakeLists.txt', 'r') as f:
        for line in f.read().split('\n'):
            line = line.strip()
            m = pattern.search(line)
            if m:
                gd = m.groupdict()
                version[gd['component']] = gd['version']
                if len(version) == 3:
                    break

    assert len(version) == 3, 'Expecting MAJOR, MINOR and RELEASE'
    return version['MAJOR'], version['MINOR'], version['RELEASE']


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        if self.is_windows():
            cmake_version = LooseVersion(
                re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    @staticmethod
    def is_windows():
        tag = platform.system().lower()
        return tag == "windows"

    @staticmethod
    def is_linux():
        tag = platform.system().lower()
        return tag == "linux"

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))

        cfg = 'Production'
        build_args = ['--config', cfg]

        cpu_count = max(2, multiprocessing.cpu_count() // 2)
        build_args += ['--', '-j{0}'.format(cpu_count)]

        root_path = get_root_path()
        build_dir = os.path.join(root_path, "build")

        # to avoid multiple build, only call reconfigure if we couldn't find the makefile
        # for python
        python_build_dir = os.path.join(build_dir, "src", "api", "python")
        if not os.path.isfile(os.path.join(python_build_dir, "Makefile")):
            args = ['--python-bindings', '--auto-download', '--lib-only']
            config_filename = os.path.join(root_path, "configure.sh")
            # call configure
            subprocess.check_call([config_filename] + args)

        # build the main library
        subprocess.check_call(
            ['cmake', '--build', '.', "--target", "cvc5"] + build_args, cwd=build_dir)

        # build the python binding
        subprocess.check_call(["make"], cwd=python_build_dir)
        # the build folder gets cleaned during the config, need to create it again
        # this is necessary since "build" is a python dist folder
        if not os.path.isdir(extdir):
            os.mkdir(extdir)

        # copy the library over. we need to consider other users that are not on linux
        # module is a directory called pycvc5
        pycvc5_module = os.path.join(python_build_dir, "pycvc5")
        dst_name = os.path.join(extdir, "pycvc5")
        if os.path.isdir(dst_name):
            # remove, then replace
            shutil.rmtree(dst_name)

        shutil.copytree(pycvc5_module, dst_name)


setup(
    name='pycvc5',
    version='.'.join(get_cvc5_version()),
    long_description='Python bindings for cvc5',
    url='https://github.com/cvc5/cvc5',
    license='BSD-3-Clause',
    zip_safe=False,
    ext_modules=[CMakeExtension('pycvc5')],
    cmdclass=dict(build_ext=CMakeBuild),
    tests_require=['pytest']
)

