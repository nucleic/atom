from dataclasses import dataclass
from typing import Optional

import pytest

from atom.api import Atom


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


# @pytest.mark.skip("Optional members with default value should be respected")
def test_optional_with_default_none_value():
    """
    Optional members with default value should be respected
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


# @pytest.mark.skip("Optional members with no default should not be allowed")
def test_optional_with_no_default():
    @dataclass
    class TestDataclass:
        x: Optional[int]

    class TestAtom(Atom):
        x: Optional[int]

    with pytest.raises(TypeError):
        TestDataclass()

    with pytest.raises(TypeError):
        TestAtom()
