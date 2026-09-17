import _io

from typing import assert_type

from atom.api import Atom, ForwardInstance, ForwardTyped, Instance, Int, List, Typed


def resolver() -> type[_io.StringIO]:
    return _io.StringIO


def resolver1() -> type[_io.StringIO]:
    return _io.StringIO


def resolver2() -> type[_io.StringIO] | type[str]:
    return _io.StringIO


def resolver3() -> type[_io.StringIO] | type[str] | type[bytes]:
    return _io.StringIO


def g() -> _io.StringIO:
    return _io.StringIO()


# Typed members infer the descriptor type from their constructor arguments while the
# instance values are the concrete runtime types.
class ATypedDefault(Atom):
    m = Typed(int)


class AForwardTypedDefault(Atom):
    m = ForwardTyped(resolver)


class AInstanceDefault(Atom):
    m = Instance((_io.StringIO, str), kwargs=dict(initial_value=str(1)))


class AForwardInstanceDefault(Atom):
    m = ForwardInstance(resolver2)


assert_type(ATypedDefault.m, Typed[int | None])
assert_type(ATypedDefault().m, int | None)
assert_type(AForwardTypedDefault.m, ForwardTyped[_io.StringIO | None])
assert_type(AForwardTypedDefault().m, _io.StringIO | None)
assert_type(AInstanceDefault.m, Instance[_io.StringIO | str])
assert_type(AInstanceDefault().m, _io.StringIO | str)
assert_type(AForwardInstanceDefault.m, ForwardInstance[_io.StringIO | str | None])
assert_type(AForwardInstanceDefault().m, _io.StringIO | str | None)


class AListTyped(Atom):
    m = Typed(List[int], optional=False)


assert_type(AListTyped.m, Typed[List[int]])
assert_type(AListTyped().m, List[int])


class AOptionalTyped(Atom):
    m = Typed(int, optional=True)


assert_type(AOptionalTyped.m, Typed[int | None])
assert_type(AOptionalTyped().m, int | None)


class AOptionalInstance(Atom):
    m = Instance(int, optional=True)


assert_type(AOptionalInstance.m, Instance[int | None])
assert_type(AOptionalInstance().m, int | None)
