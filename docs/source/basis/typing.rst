.. _basis-typing:

Typing with Atom
================

.. include:: ../substitutions.sub

Since an atom member is a Python descriptor in which the validation step is allowed to perform a type conversion (ex with `Coerced`), the types may be different when reading and writing the member value. Therefore, the type hint is logically generic over 2 types:

- the type that will be returned when accessing the member, which we will refer to as the getter or read type `T`
 
- the type that the member can be set to, which we will refer to as the setter or write type `S`


In general, the type hints shipped with Atom are sufficient to narrow down the type
of the members without requiring any manual annotation. For example:

.. code-block::

    class MyAtom(Atom):

        i = Int()
        i_f = Int(strict=False)
        l = List(int)
        d = Dict((str, bytes), List(int))

will be typed as something equivalent to:

.. code-block::

    class MyAtom(Atom):

        i: Member[int, int]
        i_f: Member[int, float]
        l: Member[TList[int], TList[int]]
        d: Member[TDict[Union[str, bytes], TList[int]], TDict[Union[str, bytes], TList[int]]]

.. note::

    Since many member names conflict with name found in the typing module we will
    add a leading `T` to types coming from typing. However in real code we recommend
    rather aliasing the Atom members with a leading `A` as illustrated in the next
    example.

However, in some cases, static typing can be more strict than Atom validation such as
for tuples and we may not want to validate at runtime the size of the tuple (even
though it may be a good idea to do so).

.. code-block::

    from typing import Tuple
    from Atom.api import Atom, Tuple as ATuple

    class MyAtom(Atom):

        t: "Member[Tuple[int, int], Tuple[int, int]]" = ATuple[int]  # type: ignore

Let's walk through this case.

.. code-block::

    from typing import Tuple
    from Atom.api import Atom, Tuple as ATuple

First, since Atom and typing share many names, one must be careful to disambiguate the
names. We recommend aliasing Atom members with a leading `A` and keeping the name from
`typing` unchanged

.. code-block::

    class MyAtom(Atom):

        t: "Member[Tuple[int, int], Tuple[int, int]]" = ATuple[int]  # type: ignore

Here we want to specify, that our tuple member is expected to store 2-tuple of int.
Since Atom does not enforce the length of a tuple, its type hint looks like
`Member[Tuple[T, ...], Tuple[T, ...]]` and makes the assumption that no fancy
type conversion occurs. If we want to go further we need a type hint and this is where
things get a bit more complicated.

Member is actually defined in C and does not actually inherit from Protocol. As a
consequence it does not implement getitem so writing `Member[int, int]` is not valid.
And there is no way to `fix` that would not involve a performance cost. This is why
we need the quote around `Member[Tuple[int, int], Tuple[int, int]]` since the type
checkers will use the definition found in the .pyi file which do define |Member| as
inheriting from protocol. The rule of thumb is ALWAYS quote annotation using Atom
members. Finally since we have annotations for the |Tuple|, type checker can infer a
type for our assignation but this one will conflict with the manual annotation hence
the need for the type ignore comment.

.. note::

    If the line becomes too long it can be split on multiple lines as follows:

    .. code-block::

        class MyAtom(Atom):

            t: "Member[Tuple[int, int], Tuple[int, int]]"
            t = ATuple[int]  # type: ignore


Similarly if one implements custom member subclasses and desire to make it compatible
with type annotations, one can define the type hint as follow but only inside a .pyi
file.

.. code-block::

    class MyMember(Member[int, str]): ...

.. note::

    Member is actually defined in C and does not actually inherit from Protocol. As
    a consequence the above syntax is only valid in .pyi file.


Member typing in term of Member[T, S]
-------------------------------------

Below we give the typing of most Atom member in term of Member to clarify the behavior
of each member with respect to typing. We also indicate their default typing, but please
note that the presence/value of some argument at the member creation will influence the
inferred type.

.. code-block::

    Value[T] = Member[T, T]             # default: Value[Any]
    Constant[T] = Member[T, NoReturn]   # default: Constant[Any]
    ReadOnly[T] = Member[T, T]          # default: ReadOnly[Any]
    Callable[T] = Member[T, T]          # default: Callable[TCallable]
    Bool[S] = Member[bool, S]           # default: Bool[bool]
    Int[S] = Member[int, S]             # default: Int[int]
    Float[S] = Member[float, S]         # default: Float[float]
    Range[S] = Member[int, S]           # default: Range[int]
    FloatRange[S] = Member[float, S]    # default: FloatRange[float]
    Bytes[S] = Member[bytes, S]         # default: Bytes[Union[bytes, str]]
    Str[S] = Member[str, S]             # default: Str[Union[bytes, str]]

    List[T] = Member[TList[T], TList[T]]
    # List() -> List[Any]
    # List(int) -> List[int]
    # List(List(int)) -> List[TList[int]]

    Set[T] = Member[TSet[T], TSet[T]]
    # Set() -> Set[Any]
    # Set(int) -> Set[int]

    Dict[KT, VT] = Member[TDict[KT, VT], TDict[KT, VT]]
    # Dict() -> Dict[Any, Any]
    # Dict(str, int) -> Dict[str, int]
    # Dict(str, List[int]) -> Dict[str, TList[int]]

    Typed[T] = Member[T, T]
    # Typed(int) -> Typed[Optional[int]]
    # Typed(int, ()) -> Typed[int]

    ForwardTyped[T] = Member[T, T]
    Instance[T] = Member[T, T]
    ForwardInstance[T] = Member[T, T]


.. note::

    |Typed|, |ForwardTyped|, |Instance| and |ForwardInstance| are special in that they
    always accept None as a valid value. However if a default handler None will never
    be seen except if it is explicitly assigned. As a consequence, those types will
    have a non-optional value when a default value constructor is provided as part of
    the member instantiation.

.. note::

    All members that can take a tuple of types as argument (List, Dict, etc) have type
    hints for up to a tuple of 3 types as argument. Supporting more types would make
    type checking even slower, so we suggest using manual annotation.

Finally the case of |Coerced| is a bit special, since we cannot teach type checker to
see a type both as a type and a callable. As a consequence for type checking to be
correct when the type itself handle the coercion the type should be manually specified
as coercer::

    c = Coerced(int, coercer=int)
