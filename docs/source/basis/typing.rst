.. _basis-typing:

Typing with Atom
================

Atom objects support both type hints and static type checking in addition to runtime validation.

Atom understands standard Python type hints, and uses them to infer the appropriate Atom members.
Type hints can also be used to specify default values.

The following example creates a class that performs run-time type checking on all instance attributes,
sets appropriate default values, and supports static type checking.

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

``typing.Optional`` attributes have a default value of ``None`` if no default is specified.

.. code-block::

    assert my_atom.num is None

Mutable default values for ``list`` and ``dict`` are OK.

.. code-block::

    assert my_atom.lst == [1, 2, 3]

The following statements will fail static type checking and cause Atom to raise a runtime ``TypeError`` exception.

.. code-block::

    my_atom.n = "Not an integer"
    my_atom.s = 5
