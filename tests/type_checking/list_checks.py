from typing import Any

from typing_extensions import assert_type

from atom.api import Atom, ContainerList, Int, List


class Z(Atom):
    pass


# List-like members are validated by checking the public descriptor type on the class
# and the concrete runtime container type on the instance. The classes below isolate the
# major inference branches directly so each case remains easy to read without relying on
# a generated fixture matrix.
class ANoDefault(Atom):
    # An untyped list member accepts any element type. The class exposes List[Any], and
    # the instance exposes the concrete list[Any] value type.
    m = List()


class ASingleType(Atom):
    # A single element type narrows the descriptor to List[int] while the instance still
    # stores a plain Python list[int].
    m = List(int)


class AUnionOfScalarTypes(Atom):
    # A tuple of accepted element types widens the list element union in the same way as
    # the equivalent scalar validation path without changing the class/instance split.
    m = List((int, float, str))


class AMemberBasedList(Atom):
    # Member-based element validation is equivalent to validating against the member's
    # concrete runtime type; Int() should therefore tighten the list to List[int].
    m = List(Int())


class ADefaultAny(Atom):
    # A default list remains valid even when no element type is declared. The descriptor
    # stays generic, but the runtime value is still a real Python list.
    m = List(default=[1])


class ADefaultTyped(Atom):
    # A default list plus an explicit element type should keep the declared element type
    # and preserve the instance value as list[int].
    m = List(int, default=[3])


class ADefaultTupleUnion(Atom):
    # Mixed default values are accepted so long as they are compatible with one of the
    # tuple members. The descriptor must still widen to the union type required by the
    # validation model.
    m = List((int, str), default=[1, str(1)])


class ADefaultMixedWithAtom(Atom):
    # Atom subclasses participate in the same union resolution as builtins. The default
    # value may include instances of Z(), but the descriptor remains the union of all valid
    # permitted element types.
    m = List((int, str, Z), default=[1, str(1), Z()])


class AContainerDefault(Atom):
    # ContainerList uses the same inference semantics as List but exposes its own
    # descriptor class on the attribute itself.
    m = ContainerList()


class AContainerTyped(Atom):
    # A typed ContainerList keeps the class-side descriptor as ContainerList[...] while the
    # instance value remains a concrete list[...] object.
    m = ContainerList((int, float))


assert_type(ANoDefault.m, List[Any])
assert_type(ANoDefault().m, list[Any])
assert_type(ASingleType.m, List[int])
assert_type(ASingleType().m, list[int])
assert_type(AUnionOfScalarTypes.m, List[int | float | str])
assert_type(AUnionOfScalarTypes().m, list[int | float | str])
assert_type(AMemberBasedList.m, List[int])
assert_type(AMemberBasedList().m, list[int])
assert_type(ADefaultAny.m, List[Any])
assert_type(ADefaultAny().m, list[Any])
assert_type(ADefaultTyped.m, List[int])
assert_type(ADefaultTyped().m, list[int])
assert_type(ADefaultTupleUnion.m, List[int | str])
assert_type(ADefaultTupleUnion().m, list[int | str])
assert_type(ADefaultMixedWithAtom.m, List[int | str | Z])
assert_type(ADefaultMixedWithAtom().m, list[int | str | Z])
assert_type(List(Int()), List[int])

# ContainerList is a distinct list-like descriptor, but the semantics match List exactly:
# the descriptor stays on the class and the actual stored value is a plain Python list.
assert_type(AContainerDefault.m, ContainerList[Any])
assert_type(AContainerDefault().m, list[Any])
assert_type(AContainerTyped.m, ContainerList[int | float])
assert_type(AContainerTyped().m, list[int | float])
assert_type(ContainerList(Int(), default=[2]), ContainerList[int])
