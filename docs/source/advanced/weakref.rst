.. _advanced-weakref:

Atom and weak references
========================

.. include:: ../substitutions.sub

Because atom objects are slotted by default and do not have an instance an
instance dictionary, they do not support weak references by defaults.

Depending on the context in which you need weak references, you have two
options:

- if you need weak references to interact with an external library (such as
  pyqt), you will need to enable the standard weakref mechanism.
- if you use weak references only internally and memory is a concern (Python
  standard weak references have a not so small overhead), you can use an
  alternative mechanism provided by atom.


Enabling default weak references
--------------------------------

In order to use the standard weak references of Python, you simply need to add
the proper slot to your object as illustrated below:

.. code-block:: python

    from atom.api import Atom

    MyWeakRefAtom(Atom):

        __slots__ = ('__weakref__',)

.. note::

  Starting with atom 0.8.0 you can use the metaclass keyword argument `enable_weakrefs`
  to achieve the same result.

  .. code-block:: python

    from atom.api import Atom

    MyWeakRefAtom(Atom, enable_weakrefs=True):
        pass


Using atom builtin weak references: |atomref|
---------------------------------------------

To create a weak reference to atom object using the builtin mechanism, you
simply have to create an instance of |atomref| using the object to reference
as argument.

In order to access the object referenced by the |atomref|, you simply need to
call it which will return the object. If the referenced object is not alive
anymore, |atomref| will return None.

.. code-block:: python

    import gc
    from atom.api import Atom, atomref

    class MyAtom(Atom):
        pass

    obj = MyAtom()
    ref = atomref(obj)

    assert obj is ref()

    del obj
    gc.collect()
    assert ref() is None
