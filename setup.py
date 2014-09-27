#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

import cppy


ext_modules = [
    Extension(
        'atom.catom',
        [
            'atom/src/atom.cpp',
            'atom/src/atommeta.cpp',
            'atom/src/callbackset.cpp',
            'atom/src/catommodule.cpp',
            'atom/src/errors.cpp',
            'atom/src/member.cpp',
            'atom/src/methodwrapper.cpp',
            'atom/src/signal.cpp',
            'atom/src/typedlist.cpp',
        ],
        include_dirs=[cppy.get_include(), 'atom/src'],
        language='c++',
    ),
]


class BuildExt(build_ext):
    """ A custom build extension for adding compiler-specific options.

    """
    c_opts = {
        'msvc': ['/EHsc']
    }

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


setup(
    name='atom',
    version='1.0.0-dev',
    author='The Nucleic Development Team',
    author_email='sccolbert@gmail.com',
    url='https://github.com/nucleic/atom',
    description='Memory efficient Python objects',
    long_description=open('README.rst').read(),
    install_requires=['distribute'],
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt},
)
