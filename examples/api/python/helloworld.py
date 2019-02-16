#!/usr/bin/env python3

#####################
#! \file helloworld.py
## \verbatim
## Top contributors (to current version):
##   Makai Mann
## This file is part of the CVC4 project.
## Copyright (c) 2009-2018 by the authors listed in the file AUTHORS
## in the top-level source directory) and their institutional affiliations.
## All rights reserved.  See the file COPYING in the top-level source
## directory for licensing information.\endverbatim
##
## \brief A very simple CVC4 tutorial example, adapted from helloworld-new.cpp
import cvc4
import kinds

if __name__ == "__main__":
    slv = cvc4.Solver()
    helloworld = slv.mkVar("Hello World!", slv.getBooleanSort())
    print(helloworld, "is", slv.checkValidAssuming(helloworld))
