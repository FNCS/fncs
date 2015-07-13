#!/usr/bin/python
import os

from distutils.core import setup
from distutils.extension import Extension

# cython is optional -- attempt import
use_cython = False
try:
    from Cython.Build import cythonize
    from Cython.Distutils import build_ext
    use_cython = True
except:
    pass

FNCS_PREFIX = "FNCS_PREFIX"
FNCS_INCLUDE = "FNCS_INCLUDE"
FNCS_LIB = "FNCS_LIB"

PREFIX = None
INCLUDE = None
LIB = None

if FNCS_PREFIX in os.environ:
    PREFIX = os.environ[FNCS_PREFIX]
    INCLUDE = PREFIX + "/include"
    LIB = PREFIX + "/lib"
else:
    INCLUDE = os.environ.get(FNCS_INCLUDE, "/usr/local/include")
    LIB = os.environ.get(FNCS_LIB, "/usr/local/lib")

sources = ["fncs.cpp"]
if use_cython:
    sources = ["fncs.pyx"]

ext_modules=[
        Extension("fncs",
            sources=sources,
            libraries=["fncs"],
            include_dirs=[INCLUDE],
            library_dirs=[LIB]
            )
        ]

if use_cython:
    ext_modules = cythonize(ext_modules, language="c++")

setup(
        name = "fncs",
        description = "Framework for Network Co-Simulation",
        author = "Jeff Daily",
        author_email = "jeff.daily@pnnl.gov",
        ext_modules = ext_modules
        )
