#####################
## setup.py
## Top contributors (to current version):
##   Makai Mann
## This file is part of the cvc5 project.
## Copyright (c) 2009-2021 by the authors listed in the file AUTHORS
## in the top-level source directory and their institutional affiliations.
## All rights reserved.  See the file COPYING in the top-level source
## directory for licensing information.
##
## This setup.py will build cvc5 using the CMake arguments in cmake_args below
## This is primarily intended for building Python wheels to be uploaded to PyPi.
## If you are installing from source, this is not the recommended way to build
## cvc5 (it does not allow you to customize the options and it will hide the
## build folder). Please instead use the main approach:
##
## ./configure.sh [your options] --python-bindings
## (since you're looking at this setup.py, you probably want python bindings)
## cd build
## make
##

import pathlib
import re

from skbuild import setup

# Get the CMake Source directory
# -- one directory up from this file
CUR_PATH=pathlib.Path(__file__).parent.absolute()
SRC_DIR=str(CUR_PATH)

# read CMakeLists.txt to get version number
version = dict()
str_pattern = 'set\(CVC5_(?P<component>MAJOR|MINOR|RELEASE)\s*(?P<version>\d+)\)'
pattern = re.compile(str_pattern)
with open(SRC_DIR + '/CMakeLists.txt', 'r') as f:
    for line in f.read().split('\n'):
        line = line.strip()
        m = pattern.search(line)
        if m:
            gd = m.groupdict()
            version[gd['component']] = gd['version']
            if len(version) == 3:
                break

if len(version) != 3:
    raise RuntimeError(
        'Unable to read version number from %s/CMakeLists.txt' % SRC_DIR)

major, minor, release = version['MAJOR'], version['MINOR'], version['RELEASE']
CVC5_VERSION = '.'.join([major, minor, release])

setup(
    name='pycvc5',
    version=CVC5_VERSION,
    long_description='Python bindings for cvc5',
    url='https://github.com/cvc5/cvc5',
    license='BSD-3-Clause',
    zip_safe=False,
    packages=['pycvc5'],
    package_dir={'pycvc5': 'src/api/'},
    cmake_args=['-DBUILD_BINDINGS_PYTHON=ON',
                '-DBUILD_LIB_ONLY=ON',
                '-DUSE_POLY=ON',
                '-DUSE_CADICAL=ON',
                '-DUSE_SYMFPU=ON',
                '-DENABLE_AUTO_DOWNLOAD=ON']
)
