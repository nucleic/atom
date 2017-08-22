#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Tests getting the version.

"""
from atom import version


def test_version():
    """Test that nothing leaks from the module.

    """
    assert hasattr(version, 'version_info')
    assert not hasattr(version, 'namedtuple')
    assert not hasattr(version, 'MAJOR')
    assert not hasattr(version, 'MINOR')
    assert not hasattr(version, 'MICRO')
