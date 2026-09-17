from typing import assert_type

from atom.api import Atom, ForwardTyped, Instance, Member, Typed


# Generic aliases are accepted as raw type arguments and should infer the same
# concrete container type as the corresponding Atom member descriptor.
class A(Atom):
    m = Typed(list[int], optional=False)


assert_type(A.m, Typed[list[int]])
assert_type(A().m, list[int])


class B(Atom):
    m = Instance((list[int], int), optional=False)


assert_type(B.m, Instance[list[int] | int])
assert_type(B().m, list[int] | int)


class C(Atom):
    m = ForwardTyped(lambda: list[int], optional=False)


assert_type(C.m, ForwardTyped[list[int]])
assert_type(C().m, list[int])


m: Member[int, str] = Member()
assert_type(m, Member[int, str])
