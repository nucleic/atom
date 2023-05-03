.. _basis-members:

Introducing the members
=======================

.. include:: ../substitutions.sub

As we have seen in the introduction, members are used in the class definition
of an atom object to define the fields that will exist on each instance of that
class. As such, members are central to atom.

The following sections will shed some lights on the different members that come
with atom and also how they work which will come handy when we will discuss
how you can customize the behaviors of members later in this guide.

.. note::

    Starting with atom 0.7, atom ships with type hints allowing type checkers to
    resolve the values behind a member. More details about how typing works in
    atom and how to add custom type hints can be found in :ref:`advanced-typing`

Member workings
---------------

From a technical point of view, members are descriptors like properties and they
can do different things when you try to access or set the attribute.

Member reading
~~~~~~~~~~~~~~

Let's first look at what happen when you access an attribute:

.. code-block:: python

    class Custom(Atom):

        value = Int()

    obj = Custom()
    obj.value
    obj.value

Since we did not pass a value for ``value`` when instantiating our class (we
did not do ``obj=Custom(value=1)``), when we first access ``value`` it does not
have any value. As a consequence the framework will fetch the default value.
As we have seen in the introduction, the default value can be specified in
several ways, either as argument to the member, or using |set_default| or even
by using a specially named method (more on that in
:ref:`basis-mangled-methods`).

Once the framework has fetched the default value  it will *validate* it. In
particular here, we are going to check that we did  get an integer for example.
The details of the validation will obviously depend on the member.

If the value is valid, next a post-validation method will be called that can
some do further processing. By default this is a no-op and we will see in
:ref:`basis-mangled-methods` how this can be customized.

With this process complete, the state of our object has changed since we
created the value stored in that instance. This corresponds to a *create* that
will be sent to the observers if any is registered.

The observer called, the value can now be stored (so that we don't go through
this again) and is now ready to be returned to the sure, the *get* step is
complete. However before doing that we will actually perform a *post-gettatr*
step. Once again this is a no-op by default but can be customized.

On further accesses, since the value exists, we will go directly retrieve the
value and perform the *post-getattr*, and no notification will be generated.

To summarize:

.. digraph:: getattr
    :align: center

    a [label="A value was previously set?"];
    a->b[label="Yes"];
    a->c[label="No"];
    b[label="get the value"];
    c[label="retrieve the default value"];
    c->d;
    d[label="validate the value"];
    d->e;
    e[label="run post-validation"];
    e->f;
    f[label="store the value"];
    f->g;
    g[label="call observers"];
    g->i;
    c->i;
    i[label="run post-getattr"];
    i->j;
    j[label="return the result of post-getattr" ];

Member writing
~~~~~~~~~~~~~~~

Setting a value follows a very similar pattern. First the value is of course
validated (and post-validated). It is then actually stored (*set*).

Next as for the *get* and *validate* operation, a *post-setattr* step is run.
As for the other *post* by default this won't do anything.

Finally is any observer is attached, the observers are notified.

To summarize:

.. digraph:: setattr
    :align: center

    a [label="validate the value"];
    a->b;
    b[label="run post-validation"];
    b->c;
    c[label="store the value"];
    c->d;
    d[label="run post-setattr"];
    d->e;
    e[label="call observers"];


Members introduction
--------------------

Now that the behavior of members is a bit less enigmatic let's introduce the
members that comes with atom.

Members for simple values
~~~~~~~~~~~~~~~~~~~~~~~~~

Atom provides the following members for basic scalars types:

- |Value|: a member that can receives any value, no validation is performed
- |Int|: an integer value. One can choose if it is allowed to cast the assigned
  values (float to int), the default is true.
- |Range|: an integer value that is clamped to fall within a range.
- |Float|: a floating point value. One can choose if it is allowed to cast the
  assigned values (int to float, ...), the default is true.
- |FloatRange|: a floating point value that is clamped to fall within a range.
- |Bytes|, |Str|: bytes and unicode strings. One can choose if it is allowed to cast the
  assigned values (str to bytes, ...), the default is false.
- |Enum|: a value that can only take a finite set of values. Note that this is
  unrelated to the enum module.

Containers and type validation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Atom also provides members for three basic containers tuple, list and
dictionaries: |Tuple|, |List|, |Dict|, |DefaultDict|. In each case, you can
specify the type of the values (key and value for dict), using members, as
follows:

.. code-block:: python

    class MyAtom(Atom):

        t = Tuple(Int())
        l = List(Float())
        d = Dict(Str(), Int())
        d = DefaultDict(Str(), Int())

Alternatively, you can pass simple Python types. In this cases they will be
wrapped in an |Instance| member that will be introduced in the next section.

.. code-block:: python

    class MyAtom(Atom):

        t = Tuple(int)
        l = List(float)
        d = Dict(str, int)
        dd = DefaultDict(str, int)

.. note::

    Note that you cannot (by default) enforce a specific number of items in
    a tuple.

.. note::

    For |DefaultDict|, the default value factory can be inferred from the value
    member. It can also be specified using the ``missing`` keyword argument.

.. note::

    In order to enforce type validation of container, atom has to use custom
    subclass. As a consequence, when assigning to a member, the original
    container is copied. This copy on assignment behavior can cause some
    surprises if you modify the original container after assigning it.

One additional important point, atom does not track the content of the
container. As a consequence, in place modifications of the container do not
trigger any notifications. One workaround can be to copy the container, modify
it and re-assign it. Another option for lists is to use a |ContainerList|
member, which uses a special list subclass sending notifications when the list
is modified.

Enforcing custom types
~~~~~~~~~~~~~~~~~~~~~~

Sticking to simple types can quickly be limiting and this is why atom
provides member to enforce that the value is simply of a certain type or a
subclass:

- |Instance|: the value must pass ``isinstance(value, types))``. Using
  |Instance| once can specify a tuple of types.
- |Typed|: the value must of the specified type or a subtypes. Only one type
  can be specified. This check is equivalent to `type(obj) in cls.mro()`. It is
  less flexible but faster than |Instance|. Use |Instance| when allowing you
  need a tuple of types or (abstract) types relying on custom
  __isinstancecheck__and |Typed| when the value type is explicit.
- |Subclass|: the value must be a class and a subclass of the specified type.

.. note ::

    By default, |Typed| and |Instance| consider ``None`` to be a valid value if
    no way to build a default value was provided. One can explicitly specify if
    ``None`` is a valid value by using the ``optional`` keyword argument when
    creating the argument. New in atom 0.7.0, previously None was always a
    valid value.

.. note::

    If a |Typed| or |Instance| member is created with ``optional=False`` and no
    mean of creating a default value (no ``args``, ``kwargs`` or ``factory``),
    trying to access the member value before setting it will result in a
    ValueError.

.. note::

    Even though, generic aliases (i.e. list[int], introduced in
    `PEP 585 <https://www.python.org/dev/peps/pep-0585/>`_ ) are not proper types they
    can be used. Note however that just like ``isinstance(a, list[int])``, a member
    ``Instance(list[int])`` does not check the type of the items of a.

In some cases, the type is not accessible when the member is instantiated
(because it will be created later in the same file for example), atom also
provides |ForwardTyped|, |ForwardInstance|, |ForwardSubclass|. Those three
members rather than taking a type or a tuple of type as argument, accept a
callable taking no argument and returning the type(s) to use for validation.

.. code-block:: python

    class Leaf(Atom):

        node = ForwardTyped(lambda : Node)

    class Node(Atom):

        parent = ForwardTyped(lambda : Node)

        leaves = List(Typed(Leaf))

In some cases, the same information may be conveniently represented either by
a custom class or something simpler, like a tuple. One example of such a use
case is a color: a color can be easily represented by the four components
(red, green, blue, alpha) but in a library may be represented by a custom
class. Atom provides the |Coerced| member to allow to enforce a particular
type while also allowing seamless conversion from alternative representations.

The conversion can occur in two ways as illustrated below:

- by calling the specified types on the provided value
- by calling an alternative coercer function provided to the member

.. code-block:: python

    class Color(object):

        def __init__(self, components):
            self.red, self.green. self.blue, self.alpha = components

    def dict_to_color(color_dict):
        components = []
        for c in ('red', 'green', 'blue', 'alpha')
            components.append(color_dict[c])
        return Color(components)

    class MyAtom(Atom):

        color = Coerced(Color)

        color2 = Coerced(Color, coercer=dict_to_color)


Memory less members
~~~~~~~~~~~~~~~~~~~

Atom also provides two members that do not remember the value they are
provided, but that can be used to fire notifications:

- |Event|: this is a member to which each time a value is assigned to, a
  notification is fired. Additionally one can specify the type of value that
  are accepted. An alternative way to fire the notification is to call the
  object you get when accessing the member.
- |Signal|: this member is similar to Qt signal. One cannot be assigned to it,
  however one can call it on instances, and when called the notifier will be
  called **with the arguments and keyword arguments passed to the signal**.
  Note that this is at odds with the general behavior of observers described
  in :ref:`basis-observation`.

  The example below illustrates how those members work:

  .. code-block:: python

    class MyAtom(Atom):

        s = Signal()

        e = Event()

        @observe('s', 'e')
        def print_value(self, change):
            print(change)

    obj = MyAtom()
    obj.e = 2
    obj.e(1)
    obj.s(2)
    obj.s.emit(1)


|Delegator|
~~~~~~~~~~~

This last member is a bit special. It does not do anything by itself but can be
used to copy the behaviors of another member. In addition, any observer
attached to the delegator will also be attached to the delegate member.

|Property|
~~~~~~~~~~

The |Property| member is a special case and it will be discussed in details
in :ref:`advanced-property`.
