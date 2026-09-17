from typing import Never, Union

from typing_extensions import assert_type

from atom.api import Atom, Property, cached_property


# Empty properties expose Never on the instance side; getter-only properties and
# property setters both carry the expected descriptor and value types.
class AEmpty(Atom):
    m = Property()


assert_type(AEmpty.m, Property[Never, Never])
assert_type(AEmpty().m, Never)


def getter(a) -> int:
    return a.b


class AGetterOnly(Atom):
    m = Property(getter)


assert_type(AGetterOnly.m, Property[int, Never])
assert_type(AGetterOnly().m, int)


def state_getter(a) -> int:
    return a._b


def state_setter(a, v: Union[int, str]) -> None:
    a._b = int(v)


def deleter(a):
    pass


class AProperty(Atom):
    m = Property(state_getter, state_setter, deleter)


assert_type(AProperty.m, Property[int, int | str])
assert_type(AProperty().m, int)


class ACached(Atom):
    @cached_property
    def m(self) -> int:
        return 1


assert_type(ACached.m, Property[int, Never])
assert_type(ACached().m, int)
