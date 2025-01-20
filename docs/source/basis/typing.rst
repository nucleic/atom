.. _basis-typing:

Using type annotations
======================

.. include:: ../substitutions.sub

.. versionadded:: 0.8.0

Atom objects support both type hints and static type checking in addition to
runtime validation.

Atom understands standard Python type hints, and uses them to infer the
appropriate Atom members. Type hints can also be used to specify default values.
Note that one can freely mix standard atom members and type annotations.

.. note::

    Str-like annotations are not supported.

The following example creates a class that performs run-time type checking on
all instance attributes, sets appropriate default values, and supports static
type checking like any other atom object.

.. code-block::

    class MyAtom(Atom):
        s: str = "Hello"
        lst: list[int] = [1, 2, 3]
        num: Optional[float]
        n = Int()

The default attribute values for each instance are set to appropriate values.

.. code-block::

    my_atom = MyAtom()
    assert my_atom.n == 0

``typing.Optional`` attributes have a default value of ``None`` if no default
is specified.

.. code-block::

    assert my_atom.num is None

Mutable default values for ``list`` and ``dict`` are OK as the default value will
be copied for each new instance.

.. code-block::

    assert my_atom.lst == [1, 2, 3]

The following statements will fail static type checking and cause Atom to raise
a runtime ``TypeError`` exception.

.. code-block::

    my_atom.n = "Not an integer"
    my_atom.s = 5

.. note::

    The above class definition is basically translated by atom into:

    .. code-block::

        class MyAtom(Atom):
            s = Str("Hello")
            lst = List(Int(), default=[1, 2, 3])
            num = Typed(float)
            n = Int()

.. note::

    By default, atom will generate runtime checks for the content of list, dict
    and set but it will not check the content of inner containers. For example
    for the annotation ``list[list[int]]``, atom will check that the provided
    list contains list but it will not check that those list contains int.

    The depth at which containers are validated is controlled by the metaclass
    keyword argument `type_containers` which default to 1. To fully omit
    validating the content of containers one can write:

    .. code-block::

        class MyAtom(Atom, type_containers=0):
            lst: list[int] = [1, 2, 3]

    Which will be equivalent at runtime to, but allow type checker to validate the
    content of the list:

    .. code-block::

        class MyAtom(Atom):
            s = Str("Hello")
            lst = List(default=[1, 2, 3])

.. versionadded:: 0.12.0

    ``Literal`` are now supported and represented using the |Enum| member.
    However ``Literal`` cannot appear in union since it is not a "real" type. If
    your union is only composed of ``Literal``, you can use a single ``Literal``
    in an equivalent manner.
