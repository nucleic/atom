.. _basis-observation:

Notifications and observers
===========================

.. include:: ../substitutions.sub

One key feature of the framework in addition to small memory footprint and type
validation is the implementation of the observer pattern. In
:ref:`basis-basis`, we introduced the notion of static observers. Here we will
discuss them in more details along with dynamic observers. We will also
describe in depth the possible signature for notification handlers and the
arguments they receive upon invocation.

.. note::

    The point at which notifications are fired has been discussed in
    :ref:`basis-members`.

Static and dynamic observers
----------------------------

An observer is a callable that is called each time a member changes. For most
members it will be:

- when the member get value for the first time either through an assignment
  or a first access when the default value is used. We will refer to this as a
  'create' event.
- whenever a different value is assigned to the member. We will refer to this
  as an 'update' event.
- when the value of a member is deleted. We will refer to this as a delete
  event.

.. note::

    The |ContainerList| member is a special case since it can emit
    notifications when elements are added or removed from the list. This will
    be referred to as 'container' events.

The distinction between static and dynamic observers comes from the moment at
which the binding of the observer to the member is defined. In the case of
static observers, this is done at the time of the class definition and hence
affects all instances of the class. On the contrary, dynamic observers are
bound to a specific instance at a later time.

The next two sections will focus on how to manage static and dynamic observers
binding, while the following sections will focus on the signature of the
handlers and the content of the notification dictionary passed to the handlers
in different situations.

Static observers
~~~~~~~~~~~~~~~~

Static observers can be bound to a member in three ways:

- declaring a method matching the name of the member to observe but whose name
  starts with ``_observe_``
- using the |observe| decorator on method. The decorator can take an
  arbitrary number of arguments which allows to tie the same observer to
  multiple members. In addition, |observe| accept as argument a dot separated
  name to automatically observe a member of an atom object stored in a member.
  Note that this mechanism is limited to a single depth (hence a single dot
  in the name).
- finally one can manage manually static observer using the following methods
  defined on the base class of all members:
  + |add_static_observer| which takes a callable and an optional flag indicating
  which change types to emit
  + |remove_static_observer| which takes a single callable as argument

Dynamic observers
~~~~~~~~~~~~~~~~~

Dynamic observers are managed using the |Atom.observe| and |Atom.unobserve|
methods of the |Atom| class. To observe one needs to pass the name of the
member to observe and the callback function. When unobserving, you can either
pass just the member name to remove all observers at once or a name and a
callback to remove specific observer.

.. note::

    Two specific members have an additional way to manage observers:

    - |Event|: expose the methods |bind| and |unbind| which takes as single
      argument the callback to bind.
    - |Signal|: similarly |Signal| exposes |connect| and |disconnect| which
      match Qt signals.


Notification handlers
---------------------

Now that we discussed all kind of observers and how to manage them, it is more
than time to discuss the expected signatures of callback and what information
the callback is passed when called.

For observers connected to all members except |Signal|, the callback should
accept a single argument which is usually called *change*. This argument is a
dictionary with ``str`` as keys which are described below:

- ``'type'``: A string describing the event that triggered the notification:
    + ``'created'``: when accessing or assigning to a member that has no previous
      value.
    + ``'update'``: when assigning a new value to a member with a previous value.
    + ``'delete'``: when deleting a member (using ``del`` or ``delattr``)
    + ``'container'``: when doing inplace modification on a the of |ContainerList|.
- ``'object'``: This is the |Atom| instance that triggered the notification.
- ``'name'``: Name of the member from which the notification originate.
- ``'value'``: New value of the member (or old value of the member in the case
  of a delete event).
- ``'oldvalue'``: Old value of the member in the case of an update.

.. note::

    As of 0.8.0 ``observe`` and  ``add_static_observer`` also accepts an optional
    ``ChangeType`` flag which can be used to selectively enable or disable
    which change ``type`` events are generated.

    .. code-block:: python

        from atom.api import Atom, Int, ChangeType

        class Widget(Atom):

            count = Int()

        def on_change(change):
            print(change["type"])

        Widget.count.add_static_observer(on_change, ChangeType.UPDATE | ChangeType.DELETE)

        w = Widget()
        w.count  # Will not emit a "create" event since it was disabled
        w.count += 1 # Will trigger an "update" event
        del w.count # Will trigger a "delete" event

.. warning::

    If you attach twice the same callback function to a member, the second call
    will override the change type flag of the observer.


In the case of ``'container'`` events emitted by |ContainerList| the change
dictionary can contains additional information (note that ``'value'`` and
``'oldvalue'`` are present):

- ``'operation'``: a str describing the operation that took place (append,
  extend, \_\_setitem\_\_, insert, \_\_delitem\_\_, pop, remove, reverse, sort,
  \_\_imul\_\_, \_\_iadd\_\_)
- ``'item'``: the item that was modified if the modification affected a single
  item.
- ``'items'``: the items that were modified if the modification affected
  multiple items.

  .. note::

    As mentioned previously, |Signal| emits notifications in a different
    format. When calling (emitting) the signal, it will pass whatever arguments
    and keyword arguments it was passed as is to the observers as illustrated
    below.

    .. code-block:: python

        class MyAtom(Atom):

            s = Signal()

        def print_pair(name, value):
            print(name, value)

        a = MyAtom()
        a.s.connect(print_pair)
        a.s('a', 1)


Suppressing notifications
-------------------------

If for any reason you need to prevent notifications to be propagated you can use
the |Atom.suppress_notifications| context manager. Inside this context manager,
notifications will not be propagated.
