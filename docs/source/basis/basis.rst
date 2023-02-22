.. _basis-basis:

Anatomy
=======

.. include:: ../substitutions.sub

Since atom is designed to allow to define compact objects, the best way to
illustrate how it works is to study a class definition making use of it. This
example will serve to introduce key concepts that will be explained in more
details in the following sections.

.. code-block:: python

    from atom.api import Atom, Value, Int, List, set_default, observe

    class CompactObject(Atom):
        """Compact object generating notifications.

        """
        untyped_value = Value()

        int_value = Int(10)

        list_value = List().tag(pref=True)

        def _post_setattr_int_value(self, old, new):
            self.untyped_value = (old, new)

        def _observe_int_value(self, change):
            print(change)

        @observe('list_value')
        def notify_change(self, change):
            print(change)


    class NewCompactObject(Atom):
        """Subclass with different default values.

        """
        list_value = default_value([1, 2])

        def _default_int_value(self):
            return 1


First note that contrary to a number of projects, atom does not export any
objects in the top level atom package. To access the publicly available names
you should import from `atom.api`.

.. code-block:: python

    from atom.api import Atom, Value, Int, List, default_value, observe

Here we import several things:

- |Atom|: This is the base class to use for all objects relying on Atom. It
  provides the some basic methods that will be described later on or in the
  API documentation. (This class inherits from a more basic class CAtom)

- |Value|, |Int|, |List|: Those are members. One can think of them as advanced
  properties (ie they are descriptors). They define the attributes that are
  available on the instances of the class. They also provide type validation.

- |observe|: This is a decorator. As we will see later, it can be used to call
  the decorated method when a member value 'change'.

- |set_default|: Members can have a default value and this object is used to
  alter it when subclassing an Atom object.

Now, that the imports are hopefully clear (or at least clearer), let's move to
the beginning of the first class definition.

.. code-block:: python

    class CompactObject(Atom):
        """Compact object generating notifications.

        """
        untyped_value = Value()

        int_value = Int(10)

        list_value = List().tag(pref=True)

Here we define a class and add to it three members. Those three members will be
the attributes, that can be manipulated on the class instances. In particular,
the following will crash while it would work for a usual python object:

.. code-block:: python

    obj = CompactObject()
    obj.non_defined = 0

This may be surprising, since on usual Python objects one can define new
attributes on instances. This limitation is the price to pay for the compacity
of Atom objects.

.. note::

    This limitation should rarely be an issue and if it is one can get dynamic
    attributes back by adding the following line to the class definition::

        __slots__ = ('__dict__',)

Ok, so each member will be one instance attribute. Now, let's look at them in
more details. Our first member is a simple |Value|. This member actaully does
not perform any type validation and can be used when the attributes can really
store anything. Our second member is an |Int|. This member will validate that
the assigned value is actually an integer and the default value is 10 instead
of 0. Finally, we have |List| which obviously can only be a list. In addition,
we tagged the member. Tags are actually metadata attached to the descriptors.
They have no built-in use in atom but they can be used to filter on an instance
members when filtering them. Refer to the
:doc:`metadata.py <../examples/ex_metadata>` example for an illustration.

.. note::

    All the available members are described in details in :ref:`basis-members`

Coming back to the class definition, we now reached the methods definitions.

.. code-block:: python

        def _post_setattr_int_value(self, old, new):
            self.untyped_value = (old, new)

        def _observe_int_value(self, change):
            print(change)

        @observe('list_value')
        def notify_change(self, change):
            print(change)

Here we define three methods. None of these are meant to be called directly by
the user-code but will be called by the framework at appropriate times.

- ``_post_setattr_int_value``:
  This function will be called right after setting the value of ``int_value``,
  as its name indicates. It will get both the value of the member before the
  setting operation (old) and the value that was just set (set).

- ``_observe_int_value``:
  This function will be called each the value of ``int_value`` changes (not
  necessarily through a setattr operation). It is passed of dictionary
  containing a bunch of information about the actual modification. We will
  describe the content of this dictionary in details in
  :ref:`basis-observation`.

- ``notify_changes``:
  Because this function is decorated with the observe decorator, it will be
  called each time ``list_value``. Note however, that changes to the container
  or its content, e.g. through ``append`` will not be caught.

.. note::

    Prefixed methods (_post_setattr, _observe, ...) are discussed in more
    details in :ref:`basis-mangled-methods`.

.. note::

    Here, we have only seen observer definition from within a class. It IS
    possible to define observers on instances and this will be discussed in
    :ref:`basis-observation`.

Now we can look at the second class definition and discuss default
values a bit more.

.. code-block:: python

    class NewCompactObject(CompactObject):
        """Subclass with different default values.

        """
        list_value = set_default([1, 2])

        def _default_int_value(self):
            return 1

In this subclass, we simply alter the default values of two of the members.
We do that in two ways:

- using |set_default| which indicates to the framework that it should create
  a copy of the member existing of the base class and change the default value.

- using a specially named method starting with ``_default_`` followed by the
  member name.

To clarify what this does, we look at what happens after we create
instances of each of our classes.

.. code-block:: python

    obj1 = CompactObject()
    print(obj1.int_value)
    print(obj1.list_value)

    obj2 = NewCompactObject()
    print(obj2.int_value)
    print(obj2.list_value)


The output of this block will be:

- ``10``: which match the specified default value in the class definition
- ``[]``: which corresponds to the absence of a specific default value for a
  list.
- ``1``: which corresponds to the value returned by the method used to compute
  the default value.
- ``[1, 2]`` which corresponds to the default value we specified using
  |set_default|.


.. note::

    First note, that even though we did not define ``__init__`` methods, we
    can pass any of the members of the class as a keyword argument, in which case
    the argument will be used to set the value of the corresponding member.

    .. code-block:: python

        obj1 = CompactObject(untyped_value='e')

.. note::

    Atom objects can be frozen using |Atom.freeze| at any time of their
    lifetime to forbid further modifications.

.. note::

    Atom objects can be pickled. Starting with atom 0.9.0 only pickleable
    members will be pickled (Constant is not pickled since it cannot be restored)
    and the fact that an object is frozen is preserved across pickling-unpickling.

.. note::

    Starting with atom 0.10.0, ``__init_subclass__`` can be used to further
    customize an Atom class. It can for example comes in handy to customize
    pickling to be limited to public members in a way that applies to all
    subclasses. See :ref:`advanced-customization` for more details.


Conclusion
----------

This brief introduction should have given some basics concerning Atom working.
The next three sections will cover in more details three points introduced
here: the members, notifications and in particular observers specific to an
instance, and finally the specially named methods used to alter default
member behaviors.

.. note::

  Starting with atom 0.8.0 atom classes can also infer their members from type
  annotations see :ref:`basis-typing`
