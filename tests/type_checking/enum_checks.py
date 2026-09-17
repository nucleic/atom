from typing_extensions import assert_type

from atom.api import Atom, Enum


# Enum descriptors hold the member kind on the class while the instance sees the
# specific enum value; helper methods may widen the enum value union.
e = Enum(1, 2)


class A(Atom):
    e1 = e
    e2 = e("1")
    e3 = e.added("1")
    e4 = e.removed(2)


assert_type(A.e1, Enum[int])
assert_type(A.e2, Enum[int | str])
assert_type(A.e3, Enum[int | str])
assert_type(A.e4, Enum[int])
