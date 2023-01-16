from typing import Generic, Optional, Type, TypeVar, Union, overload

from atom.api import Atom, Coerced, Dict, Instance, Int, Member, Subclass, Typed


def coercer(a: Union[list, int]) -> list:
    pass


class A(Atom):

    a = Int().tag(a=1)

    t = Typed(float)
    t2 = Typed(float, (0.0,))
    t2 = Typed(float, factory=lambda: 1.0)

    d = Dict()
    d1 = Dict(Instance((int, str)), Instance((int, str)))
    d2 = Dict((int,))
    d3 = Dict((int, float))
    d4 = Dict(int, Typed(int))
    d5 = Dict(None, Dict(int))

    c: Coerced[list, Union[list, int]] = Coerced(list)  # type: ignore
    c1 = Coerced(list, coercer=coercer)

    s = Subclass(int)


reveal_type(A.a)
reveal_type(A.t)
reveal_type(A.t2)
# reveal_type(A.d)
# reveal_type(A.d1)
# reveal_type(A.d2)
# reveal_type(A.d3)
# reveal_type(A.d4)
# reveal_type(A.d5)
reveal_type(A.c)
reveal_type(A.c1)
reveal_type(A.s)


test = A()
reveal_type(test.a)
reveal_type(test.t)
reveal_type(test.t2)
# reveal_type(test.d)
# reveal_type(test.d1)
# reveal_type(test.d2)
# reveal_type(test.d3)
# reveal_type(test.d4)
# reveal_type(test.d5)
reveal_type(test.c)
reveal_type(test.c1)
reveal_type(test.s)
reveal_type(test.d1)


class i(int):
    pass


test.c1 = 1
test.s = i
