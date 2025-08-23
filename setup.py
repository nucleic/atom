# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2025, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------

from setuptools import Extension, setup

try:
    from cppy import CppyBuildExt
except ImportError as e:
    raise RuntimeError(
        "Missing setup required dependencies: cppy. "
        "Installing through pip as recommended ensure one never hits this issue."
    ) from e

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
