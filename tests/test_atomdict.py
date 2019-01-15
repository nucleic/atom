#------------------------------------------------------------------------------
# Copyright (c) 2018, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the typed dictionary.


"""
import sys
import pytest

from atom.api import Atom, Dict, Int


@pytest.fixture
def atom_dict():
    """Atom with different Dict members.

    """
    class DictAtom(Atom):
        untyped = Dict()
        keytyped = Dict(Int())
        valuetyped = Dict(value=Int())
        fullytyped = Dict(Int(), Int())

    return DictAtom()

@pytest.mark.parametrize('member',
                         ['untyped', 'keytyped', 'valuetyped', 'fullytyped'])
def test_repr(atom_dict, member):
    """Test the repr.

    """
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert repr(getattr(atom_dict, member)) == repr(d)


@pytest.mark.parametrize('member',
                         ['untyped', 'keytyped', 'valuetyped', 'fullytyped'])
def test_len(atom_dict, member):
    """Test the len.

    """
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert len(getattr(atom_dict, member)) == len(d)


@pytest.mark.parametrize('member',
                         ['untyped', 'keytyped', 'valuetyped', 'fullytyped'])
def test_contains(atom_dict, member):
    """Test __contains__.

    """
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert 5 in getattr(atom_dict, member)
    del getattr(atom_dict, member)[5]
    assert 5 not in getattr(atom_dict, member)

@pytest.mark.parametrize('member',
                         ['untyped', 'keytyped', 'valuetyped', 'fullytyped'])
def test_keys(atom_dict, member):
    """Test the keys.

    """
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert getattr(atom_dict, member).keys() == d.keys()


@pytest.mark.parametrize('member',
                         ['untyped', 'keytyped', 'valuetyped', 'fullytyped'])
def test_copy(atom_dict, member):
    """Test copy.

    """
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert getattr(atom_dict, member).copy() == d

@pytest.mark.skipif(sys.version_info > (3,), reason='Meaningful only on Python 2')
@pytest.mark.parametrize('member',
                         ['untyped', 'keytyped', 'valuetyped', 'fullytyped'])
def test_has_key(atom_dict, member):
    """Test has_key.

    """
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert getattr(atom_dict, member).has_key(5)


def test_setitem(atom_dict):
    """Test setting items.

    """
    atom_dict.untyped[''] = 1

    atom_dict.keytyped[1] = ''
    with pytest.raises(TypeError):
        atom_dict.keytyped[''] = 1

    atom_dict.valuetyped[1] = 1
    with pytest.raises(TypeError):
        atom_dict.valuetyped[''] = ''

    atom_dict.fullytyped[1] = 1
    with pytest.raises(TypeError):
        atom_dict.fullytyped[''] = 1
    with pytest.raises(TypeError):
        atom_dict.fullytyped[1] = ''
