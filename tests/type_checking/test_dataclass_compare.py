from dataclasses import dataclass
from dataclasses import field
from typing import Optional

import pytest

from atom.api import Atom
from atom.api import List
from atom.instance import Instance


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


def test_constructor() -> None:
    atm = InventoryDataclass(name="widget1", unit_price=0.99, quantity_on_hand=10, quantity_sold=1)
    dcl = InventoryAtom(name="widget1", unit_price=0.99, quantity_on_hand=10, quantity_sold=1)

    assert isinstance(atm.name, str)
    assert isinstance(atm.unit_price, float)
    assert isinstance(atm.quantity_on_hand, int)
    assert isinstance(atm.quantity_sold, int)

    assert isinstance(dcl.name, str)
    assert isinstance(dcl.unit_price, float)
    assert isinstance(dcl.quantity_on_hand, int)
    assert isinstance(dcl.quantity_sold, int)


def test_default_value() -> None:
    dcl = InventoryDataclass(name="widget1", unit_price=0.99, quantity_sold=1)
    atm = InventoryAtom(name="widget1", unit_price=0.99, quantity_sold=1)

    assert atm.quantity_on_hand == 2
    assert dcl.quantity_on_hand == 2


def test_optional_with_default_none_value() -> None:
    """Optional members with default value should be respected"""

    @dataclass
    class TestDataclass:
        x: Optional[int] = None

    class TestAtom(Atom):
        x: Optional[int] = None

    dcl = TestDataclass()
    assert dcl.x is None

    atm = TestAtom()
    assert atm.x is None


def test_no_default() -> None:
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
        TestDataclass()  # type: ignore

    atm = TestAtom()
    assert atm.x == 0


def test_optional_with_no_default() -> None:
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
        TestDataclass()  # type: ignore

    atm = TestAtom()
    assert atm.x is None


def test_list() -> None:
    """Demonstrate list validation

    Simple demonstration of Atom's validation of list contents.
    """

    @dataclass
    class TestDataclass:
        x: list[int]

    class TestAtom(Atom):
        x: list[int]

    TestDataclass(x=[1, 2, 3])
    TestAtom(x=[1, 2, 3])

    TestDataclass(x=[1, "a", 3])  # type: ignore
    with pytest.raises(TypeError):
        TestAtom(x=[1, "a", 3])


def test_list_default() -> None:
    """Mutable Defaults

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


def test_default_list_factory() -> None:
    """Default list factory"""

    # Dataclass example
    @dataclass
    class TestDataclassNoMutableDefault:
        x: list[int] = field(default_factory=list)

    dcl = TestDataclassNoMutableDefault()
    assert dcl.x == []

    class TestAtom2(Atom):
        x: list[int] = Instance(list[int], factory=list)  # type: ignore

    atm = TestAtom2()
    assert atm.x == []

    with pytest.raises(TypeError):
        TestAtom2(x=[1, "a", 3])


def test_default_list_factory_with_default() -> None:
    """This example demonstrates how an Atom member can be used to
    avoid the expense of run-time checking while relying on
    type-checkers to enforce assignments.

    """

    class TestAtom1(Atom):
        x: list[int] = List(default=[1, 2, 3])  # type: ignore

    atm = TestAtom1()
    assert atm.x == [1, 2, 3]

    atm = TestAtom1(x=[1, "a", 3])


def test_instance_without_default() -> None:
    class Atom1(Atom):
        x: int

    class Atom2(Atom):
        y: Atom1

    assert isinstance(Atom2(y=Atom1()).y, Atom1)

    with pytest.raises(TypeError):
        Atom2(y="notAnAtom1")

    # FAILS:
    assert isinstance(Atom2().y, Atom1)


def test_optional_instance_with_constructor_arguments() -> None:
    class Atom1(Atom):
        x: int

    # Default factory specification
    class Atom2(Atom):
        y: Optional[Atom1] = Instance(Atom1, kwargs={"x": 2})  # type: ignore

    assert Atom2().y.x == 2  # type: ignore

    with pytest.raises(TypeError):
        Atom2().y = None
