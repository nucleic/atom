from dataclasses import dataclass
from dataclasses import field
from typing import Optional

import pytest

from atom.api import Atom
from atom.api import List
from atom.api import Typed


class InventoryAtom(Atom):
    """Class for keeping track of an item in inventory."""
    name: str
    unit_price: float
    quantity_sold: Optional[int]
    quantity_on_hand: int = 2


@dataclass
class InventoryDataclass:
    """Class for keeping track of an item in inventory."""
    name: str
    unit_price: float
    quantity_sold: Optional[int]
    quantity_on_hand: int = 2


def test_constructor():
    atm = InventoryDataclass(name='widget1', unit_price=0.99, quantity_on_hand=10, quantity_sold=1)
    dcl = InventoryAtom(name='widget1', unit_price=0.99, quantity_on_hand=10, quantity_sold=1)

    for obj in [atm, dcl]:
        assert isinstance(obj.name, str)
        assert isinstance(obj.unit_price, float)
        assert isinstance(obj.quantity_on_hand, int)
        assert isinstance(obj.quantity_sold, int)


def test_default_value():
    dcl = InventoryDataclass(name='widget1', unit_price=0.99, quantity_sold=1)
    atm = InventoryAtom(name='widget1', unit_price=0.99, quantity_sold=1)

    assert atm.quantity_on_hand == 2
    assert dcl.quantity_on_hand == 2


def test_optional_with_default_none_value():
    """Optional members with default value should be respected

    """

    @dataclass
    class TestDataclass:
        x: Optional[int] = None

    class TestAtom(Atom):
        x: Optional[int] = None

    dcl = TestDataclass()
    assert dcl.x is None

    atm = TestAtom()
    assert atm.x is None


def test_no_default():
    """
    This test demonstrates a difference between Dataclasses and standard Atom behavior

    When a member/field is declared with no default:

    - Atom always sets a default value (e.g. 0 for `int` type), and the field is not required in the constructor.
    - A dataclass requires the field in the constructor, otherwise a TypeError is raised.

    """

    @dataclass
    class TestDataclass:
        x: int

    class TestAtom(Atom):
        x: int

    with pytest.raises(TypeError):
        dcl = TestDataclass()

    atm = TestAtom()
    assert atm.x == 0


def test_optional_with_no_default():
    """
        This test demonstrates a difference between Dataclasses and standard Atom behavior

    When a member/field is declared as Optional without a default:

    - Atom sets the default value of an Optional to None, and the field is not required in the constructor.
    - A dataclass requires the field in the constructor, otherwise a TypeError is raised.

    """

    @dataclass
    class TestDataclass:
        x: Optional[int]

    class TestAtom(Atom):
        x: Optional[int]

    with pytest.raises(TypeError):
        TestDataclass()

    atm = TestAtom()
    assert atm.x is None


def test_list():
    """ Demonstrate list validation

    Simple demonstration of Atom's validation of list contents.
    """

    @dataclass
    class TestDataclass:
        x: list[int]

    class TestAtom(Atom):
        x: list[int]

    TestDataclass(x=[1, 2, 3])
    TestAtom(x=[1, 2, 3])

    TestDataclass(x=[1, 'a', 3])
    with pytest.raises(TypeError):
        TestAtom(x=[1, 'a', 3])


def test_list_default():
    """ Mutable Defaults

    This test demonstrates a difference between Dataclasses and standard Atom behavior

    Dataclasses do not allow mutable default values.
    """

    with pytest.raises(ValueError):
        @dataclass
        class TestDataclassNoMutableDefault:
            x: list[int] = [1, 2, 3]

    class TestAtom(Atom):
        x: list[int] = [1, 2, 3]

    atm = TestAtom()
    assert atm.x == [1, 2, 3]

    atm = TestAtom(x=[1])
    assert atm.x == [1]


def test_default_list_factory():
    """ Default list factory

    """
    # Dataclass example
    @dataclass
    class TestDataclassNoMutableDefault:
        x: list[int] = field(default_factory=list)

    dcl = TestDataclassNoMutableDefault()
    assert dcl.x == []

    class TestAtom2(Atom):
        x: list[int] = Typed(list, factory=list)

    atm = TestAtom2()
    assert atm.x == []

    with pytest.raises(TypeError):
        TestAtom2(x=[1, 'a', 3])


def test_default_list_factory_with_default():
    class TestAtom1(Atom):
        x: list[int] = List(default=[1, 2, 3])

    atm = TestAtom1()
    assert atm.x == [1, 2, 3]

    with pytest.raises(TypeError):
        TestAtom1(x=[1, 'a', 3])
