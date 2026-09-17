from typing import Type

from typing_extensions import assert_type

from atom.api import Atom, ForwardSubclass, Subclass


# Subclass members carry the class object descriptor on the class and the concrete
# class type on the instance, including unioning the accepted classes.
class A:
    pass


class B(A):
    pass


class T(Atom):
    m = Subclass(A)


assert_type(T.m, Subclass[type[A]])
assert_type(T().m, type[A])


class T2(Atom):
    m = Subclass((int, A), B)


assert_type(T2.m, Subclass[type[int] | type[A]])
assert_type(T2().m, type[int] | type[A])


def resolve1() -> Type[A]:
    return A


def resolve2() -> tuple[Type[A]]:
    return (A,)


def resolve3() -> tuple[Type[int], Type[A]]:
    return (int, A)


class T3(Atom):
    m = ForwardSubclass(resolve3)


assert_type(T3.m, ForwardSubclass[type[int] | type[A]])
assert_type(T3().m, type[int] | type[A])
