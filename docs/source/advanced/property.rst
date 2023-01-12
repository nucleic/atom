.. _advanced-property:

The Property member
===================

.. include:: ../substitutions.sub

The |Property| member looks a lot like a normal Python property which makes it
quite different from other members. In particular because there is no way from
atom to know when the value returned by a |Property| changes, **no
notifications are emitted by default when getting or setting it**.

Defining a Property member
--------------------------

Defining a |Property| and the getter, setter and deleter associated to it can
be done in several equivalent manners illustrated below:

.. code-block:: Python

    from atom.api import Atom, Property, Value

    def get_v(owner):
        return owner.v

    def set_v(owner, value):
        owner.v = value

    def del_v(owner):
        del owner.v

    class MyAtom(Atom):

        v = Value()

        p1 = Property(get_v, set_v, del_v)

        p2 = Property()

        def _get_p2(self):
            return get_v(self)

        def _set_p2(self, value):
            set_v(self, value

        def _del_p2(self):
            del_v(self)

        p3 = Property()

        p3.getter
        def _get(self):
            return get_v(self)

        p3.setter
        def _set(self, value):
            set_v(self, value)

        p3.deleter
        def _del(self):
            del_v(self)


Cached properties
-----------------

For **read-only** properties, atom offers the option to cache the value
returned by the getter. This can be convenient if the getter performs an
expensive operation. The cache can be reset at a later time by deleting the
property or by calling the :py:meth:`atom.property.Property.reset` method of
the member as illustrated below:

.. code-block:: python

    from atom.api import Atom, Property, cached_property

    class MyAtom(Atom):

        cp1 = Property(cached=True)

        def _get_cp1(self):
            print('Called cp1')

        @cached_property
        def cp2(self):
            print('Called cp2')

    a = MyAtom()

    a.cp1
    a.cp1
    del a.cp1
    a.cp1

    a.cp2
    a.cp2
    MyAtom.cp2.reset(a)
    a.cp2

Running this code will print "Called cp1/2" only twice each.


Notifications from a Property
-----------------------------

As mentionned in the introduction, |Property| does not fire notifications
upon get/setattr. However it will always fire notifications upon
deletion/reset. To manually fire notifications from a
property, please refer to :ref:`advanced-manual-notifications`.
