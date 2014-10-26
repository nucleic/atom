#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

import cppy


ext_modules = [
    Extension(
        'atom.catom',
        [
            'src/atom.cpp',
            'src/atommeta.cpp',
            'src/callbackset.cpp',
            'src/catommodule.cpp',
            'src/errors.cpp',
            'src/member.cpp',
            'src/memberchange.cpp',
            'src/methodwrapper.cpp',
            'src/signal.cpp',
            'src/typeddict.cpp',
            'src/typedlist.cpp',
            'src/typedset.cpp',
        ],
        include_dirs=[cppy.get_include(), 'src'],
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
    author='Nucleic',
    author_email='sccolbert@gmail.com',
    url='https://github.com/nucleic/atom',
    description='Memory efficient Python objects',
    long_description=open('README.rst').read(),
    license='3-Clause BSD',
    install_requires=['six'],
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt},
)
