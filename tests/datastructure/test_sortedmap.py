#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the sortedmap that acts like an ordered dictionary.

"""
import pytest
from atom.datastructures.api import sortedmap


def test_sortedmap():
    """Test sortedmap methods and protocols.

    """
    smap = sortedmap()
    smap['a'] = 1
    smap['b'] = 2
    smap['c'] = 3
    assert 'a' in smap

    # Test get method
    assert smap.get('a') == 1
    assert smap.get('d') is None
    assert smap.get('e', 4) == 4

    # Test that we get the keys and values in the proper order
    assert smap.keys() == ['a', 'b', 'c']
    assert smap.values() == [1, 2, 3]
    assert smap.items() == [('a', 1), ('b', 2), ('c', 3)]

    # Test popping a key and check the order
    assert smap.pop('b') == 2
    assert smap.keys() == ['a', 'c']

    # Test iterating and checking the order
    smap['d'] = 4
    assert [k for k in smap] == ['a', 'c', 'd']

    # Test the repr
    assert "sortedmap" in repr(smap)
    assert eval(repr(smap)) == smap

    # Test deleting a key
    del smap['c']
    assert smap.keys() == ['a', 'd']

    # Test copying
    csmap = smap.copy()
    assert csmap is not smap
    assert csmap == smap

    # Test sizeof
    smap.sizeof()

    # Test clear
    smap.clear()
    assert not smap


def test_handling_bad_arguments():
    """Test handling bad arguments to sortedmap methods.

    """
    smap = sortedmap()
    # Test bad parameters for get
    with pytest.raises(TypeError):
        smap.get()
    with pytest.raises(TypeError):
        smap.get('r', None, None)

    # Test bad parameters for pop
    with pytest.raises(TypeError):
        smap.pop()
    with pytest.raises(TypeError):
        smap.pop(None, None)
