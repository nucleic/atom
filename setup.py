# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import os

from setuptools import Extension, setup

try:
    from cppy import CppyBuildExt
except ImportError as e:
    raise RuntimeError(
        "Missing setup required dependencies: cppy. "
        "Installing through pip as recommended ensure one never hits this issue."
    ) from e

# Use the env var ATOM_DISABLE_FH4 to disable linking against VCRUNTIME140_1.dll
if "ATOM_DISABLE_FH4" in os.environ:
    os.environ.setdefault("CPPY_DISABLE_FH4", "1")

ext_modules = [
    Extension(
        "atom.catom",
        [
            "atom/src/atomlist.cpp",
            "atom/src/atomdict.cpp",
            "atom/src/atomset.cpp",
            "atom/src/atomref.cpp",
            "atom/src/catom.cpp",
            "atom/src/catommodule.cpp",
            "atom/src/defaultvaluebehavior.cpp",
            "atom/src/delattrbehavior.cpp",
            "atom/src/enumtypes.cpp",
            "atom/src/eventbinder.cpp",
            "atom/src/getattrbehavior.cpp",
            "atom/src/getstatebehavior.cpp",
            "atom/src/member.cpp",
            "atom/src/memberchange.cpp",
            "atom/src/methodwrapper.cpp",
            "atom/src/observerpool.cpp",
            "atom/src/postgetattrbehavior.cpp",
            "atom/src/postsetattrbehavior.cpp",
            "atom/src/postvalidatebehavior.cpp",
            "atom/src/propertyhelper.cpp",
            "atom/src/setattrbehavior.cpp",
            "atom/src/signalconnector.cpp",
            "atom/src/validatebehavior.cpp",
        ],
        include_dirs=["src"],
        language="c++",
    ),
    Extension(
        "atom.datastructures.sortedmap",
        ["atom/src/sortedmap.cpp"],
        include_dirs=["src"],
        language="c++",
    ),
]


setup(
    use_scm_version=True,
    ext_modules=ext_modules,
    cmdclass={"build_ext": CppyBuildExt},
)
