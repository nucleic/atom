from typing_extensions import assert_type

from atom.api import Atom, Delegator, Int


# Delegator retains the descriptor wrapper on the class but exposes the delegated
# value on the instance.
class A(Atom):
    i = Int(strict=False)
    m = Delegator(i)


assert_type(A.m, Delegator[int, int | float])
assert_type(A().m, int)
