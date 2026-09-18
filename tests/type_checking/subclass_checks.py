from typing import Type, assert_type

from atom.api import Atom, ForwardSubclass, Subclass


# Subclass members carry the class object descriptor on the class and the concrete
# class type on the instance, including unioning the accepted classes.
class A:
    pass


class B(A):
    pass


class T(Atom):
    m = Subclass(A)


# ty treats the class-level descriptor and instance-level value as a stricter
# `type[type[...]]` exactness boundary than mypy/pyrefly, so the semantic checks are
# intentionally ignored for ty until the upstream checker matches the runtime model.
assert_type(T.m, Subclass[type[A]])  # ty: ignore[type-assertion-failure]
assert_type(T().m, type[A])  # ty: ignore[type-assertion-failure]


class T2(Atom):
    m = Subclass((int, A), B)


assert_type(T2.m, Subclass[type[int | A]])  # ty: ignore[type-assertion-failure]
assert_type(T2().m, type[int | A])  # ty: ignore[type-assertion-failure]


def resolve1() -> Type[A]:
    return A


def resolve2() -> tuple[Type[A]]:
    return (A,)


class T3(Atom):
    m = ForwardSubclass(resolve1)


assert_type(T3.m, ForwardSubclass[type[A]])  # ty: ignore[type-assertion-failure]
assert_type(T3().m, type[A])  # ty: ignore[type-assertion-failure]


class T4(Atom):
    m = ForwardSubclass(resolve2)


assert_type(T4.m, ForwardSubclass[type[A]])  # ty: ignore[type-assertion-failure]
assert_type(T4().m, type[A])  # ty: ignore[type-assertion-failure]


def resolve3() -> tuple[Type[int], Type[A]]:
    return (int, A)


class T5(Atom):
    m = ForwardSubclass(resolve3)


assert_type(T5.m, ForwardSubclass[type[int | A]])  # ty: ignore[type-assertion-failure]
assert_type(T5().m, type[int | A])  # ty: ignore[type-assertion-failure]
