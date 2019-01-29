#------------------------------------------------------------------------------
# Copyright (c) 2013-2019, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
import os
import sys
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

sys.path.insert(0, os.path.abspath('.'))
from atom.version import __version__

ext_modules = [
    Extension(
        'atom.catom',
        [
            'atom/src/atomlist.cpp',
            'atom/src/atomref.cpp',
            'atom/src/catom.cpp',
            'atom/src/catommodule.cpp',
            'atom/src/defaultvaluebehavior.cpp',
            'atom/src/delattrbehavior.cpp',
            'atom/src/enumtypes.cpp',
            'atom/src/eventbinder.cpp',
            'atom/src/getattrbehavior.cpp',
            'atom/src/member.cpp',
            'atom/src/memberchange.cpp',
            'atom/src/methodwrapper.cpp',
            'atom/src/observerpool.cpp',
            'atom/src/postgetattrbehavior.cpp',
            'atom/src/postsetattrbehavior.cpp',
            'atom/src/postvalidatebehavior.cpp',
            'atom/src/propertyhelper.cpp',
            'atom/src/setattrbehavior.cpp',
            'atom/src/signalconnector.cpp',
            'atom/src/validatebehavior.cpp',
        ],
        language='c++',
    ),
    Extension(
        'atom.datastructures.sortedmap',
        ['atom/src/sortedmap.cpp'],
        language='c++',
    ),
]


class BuildExt(build_ext):
    """ A custom build extension for adding compiler-specific options.

    """
    c_opts = {
        'msvc': ['/EHsc']
    }

    def initialize_options(self):
        build_ext.initialize_options(self)
        self.debug = False

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


setup(
    name='atom',
    version=__version__,
    author='The Nucleic Development Team',
    author_email='sccolbert@gmail.com',
    url='https://github.com/nucleic/atom',
    description='Memory efficient Python objects',
    long_description=open('README.rst').read(),
    license='BSD',
    classifiers=[
          # https://pypi.org/pypi?%3Aaction=list_classifiers
          'Programming Language :: Python',
          'Programming Language :: Python :: 2',
          'Programming Language :: Python :: 2.7',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: 3.4',
          'Programming Language :: Python :: 3.5',
          'Programming Language :: Python :: 3.6',
          'Programming Language :: Python :: 3.7',
          'Programming Language :: Python :: Implementation :: CPython',
      ],
    python_requires='>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, !=3.3.*',
    install_requires=['setuptools'],
    packages=find_packages(exclude=['tests', 'tests.*']),
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt},
)
