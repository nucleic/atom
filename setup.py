#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext


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

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


setup(
    name='atom',
    version='0.3.9',
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
