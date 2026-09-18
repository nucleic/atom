from typing import Any, assert_type

from atom.api import Atom, Dict, Int


# Dict members are checked along both axes: the class exposes Dict[key_type, value_type]
# and the instance exposes dict[key_type, value_type]. The classes below cover the main
# key/value inference combinations without depending on a generated fixture matrix.
class AUnspecified(Atom):
    # No key or value constraints means the descriptor stays completely generic.
    m = Dict()


class ADefaultDict(Atom):
    # The default value is only a factory for instance state; it does not widen the type
    # declaration itself.
    m = Dict(default={"a": 1})


class AKeyScalar(Atom):
    # Key-only typing narrows only the key side and leaves the value side free.
    m = Dict(int)


class AKeyTuple(Atom):
    # A tuple of key types creates a key union and should propagate to the descriptor.
    m = Dict((int, float, str))


class AValueScalar(Atom):
    # Value-only typing narrows only the value side while the key remains unconstrained.
    m = Dict(None, int)


class AValueTuple(Atom):
    # Tuple value types widen to the same union semantics used elsewhere in Atom.
    m = Dict(None, (int, float, str))


class AKeywordValue(Atom):
    # The keyword form is equivalent to positional value specification; it still narrows
    # only the value side.
    m = Dict(value=(int, float, str))


class AKeyValueScalar(Atom):
    # Fully specified key/value typing should produce the precise Dict[int, int] result.
    m = Dict(int, int)


class AKeyValueTuple(Atom):
    # Tuple-based keys and values produce union inference on both sides of the dictionary.
    m = Dict((int, str), (int, float))


class AKeyValueTripleTuple(Atom):
    # Three-way key and value unions keep the same shape but widen the key/value unions to
    # their combined set of valid types.
    m = Dict((int, str, bytes), (int, float, str))


class AMemberKey(Atom):
    # Member-based key specification should resolve to the member's concrete type and keep
    # the value side typed independently.
    m = Dict(Int(), (int, float))


assert_type(AUnspecified.m, Dict[Any, Any])
assert_type(AUnspecified().m, dict[Any, Any])
assert_type(ADefaultDict.m, Dict[Any, Any])
assert_type(ADefaultDict().m, dict[Any, Any])
assert_type(AKeyScalar.m, Dict[int, Any])
assert_type(AKeyScalar().m, dict[int, Any])
assert_type(AKeyTuple.m, Dict[int | float | str, Any])
assert_type(AKeyTuple().m, dict[int | float | str, Any])
assert_type(AValueScalar.m, Dict[Any, int])
assert_type(AValueScalar().m, dict[Any, int])
assert_type(AValueTuple.m, Dict[Any, int | float | str])
assert_type(AValueTuple().m, dict[Any, int | float | str])
assert_type(AKeywordValue.m, Dict[Any, int | float | str])
assert_type(AKeywordValue().m, dict[Any, int | float | str])
assert_type(AKeyValueScalar.m, Dict[int, int])
assert_type(AKeyValueScalar().m, dict[int, int])
assert_type(AKeyValueTuple.m, Dict[int | str, int | float])
assert_type(AKeyValueTuple().m, dict[int | str, int | float])
assert_type(AKeyValueTripleTuple.m, Dict[int | str | bytes, int | float | str])
assert_type(AKeyValueTripleTuple().m, dict[int | str | bytes, int | float | str])
assert_type(AMemberKey.m, Dict[int, int | float])
assert_type(AMemberKey().m, dict[int, int | float])


# The additional classes below exercise the single-element tuple variants and the member-
# based key/value shorthand that are easy to miss in a manual suite. They are redundant in
# meaning but important as coverage for the constructor overloads.
class AKeyOneTuple(Atom):
    # A one-element tuple key is equivalent to the scalar key type case and should collapse
    # to a single key type.
    m = Dict((int,), int)


class AKeyTwoTuple(Atom):
    # Two-element tuple keys widen the dictionary key union to the union of both member
    # types while the value side remains fixed.
    m = Dict((int, str), int)


class AKeyThreeTuple(Atom):
    # Three-element tuple keys widen to the three-way union while preserving the value
    # typing contract.
    m = Dict((int, str, bytes), int)


class AValueFromIntMember(Atom):
    # Value-side Int() members should infer as int exactly like the scalar int case.
    m = Dict(None, Int())


class AKeywordValueFromIntMember(Atom):
    # The keyword value form with an Int() member is covered separately to ensure the
    # overload resolution remains the same as the positional equivalent.
    m = Dict(value=Int())


class AMemberAsKeyAndValue(Atom):
    # Using a member on both sides verifies that the key and value validation paths each
    # resolve to the member's concrete type without cross-contaminating the other side.
    m = Dict(Int(), Int())


assert_type(AKeyOneTuple.m, Dict[int, int])
assert_type(AKeyOneTuple().m, dict[int, int])
assert_type(AKeyTwoTuple.m, Dict[int | str, int])
assert_type(AKeyTwoTuple().m, dict[int | str, int])
assert_type(AKeyThreeTuple.m, Dict[int | str | bytes, int])
assert_type(AKeyThreeTuple().m, dict[int | str | bytes, int])
assert_type(AValueFromIntMember.m, Dict[Any, int])
assert_type(AValueFromIntMember().m, dict[Any, int])
assert_type(AKeywordValueFromIntMember.m, Dict[Any, int])
assert_type(AKeywordValueFromIntMember().m, dict[Any, int])
assert_type(AMemberAsKeyAndValue.m, Dict[int, int])
assert_type(AMemberAsKeyAndValue().m, dict[int, int])
