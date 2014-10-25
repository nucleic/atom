#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
""" Atom is a library for creating memory efficient Python objects.

Versioning follows the http://semver.org/ specification.

"""
__major_version__ = 1
__minor_version__ = 0
__patch_version__ = 0
__build_version__ = 'dev'


__version_info__ = (__major_version__, __minor_version__, __patch_version__)


__version__ = '%s.%s.%s' % __version_info__ + (
    ('-' + __build_version__) if __build_version__ else ''
)
