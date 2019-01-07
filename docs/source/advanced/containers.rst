.. _advanced-containers:

Atom's containers
=================

.. include:: ../substitutions.sub

Atom uses custom containers to implement type validation and notifications. It
implements two list subclasses and one dictionary subclass to this effect.

.. note::

    Currently the typed validated dictionary is not a subclass of the Python
    builtin dictionary type. This can cause some unexpected issues in
    particular when assigning the value stored in a |Dict| member to another
    |Dict| which will fail. To circumvent this issue one should call ``dict``
    on the content of the first member.
    This is a known problem and will be fixed in a future version of atom.

Usually, users should not instantiate those containers manually, in particular
because they need a reference to both the member and the instance to which
they are tied.

Atom provides however an alternative mapping type which uses less memory than
a regular dictionary in particular when the mapping contain only few objects:
|sortedmap|.


|sortedmap|
-----------

|sortedmap| can be imported from ``atom.datastructures.api``. Contrary to a
regular Python dictionary, |sortedmap| does not requires the keys to be
hashable, however they should be sortable. |sortedmap| will fall back on the
Python 2 behavior to order any Python object based on the class name and the
object id if two objects cannot be compared otherwise.

In terms of memory efficiency, here is a quick comparison:
+-------------+-------------+---------------+
|             |  dict       | sortedmap     |
+=============+=============+===============+
| empty       | 240         | 72            |
+-------------+-------------+---------------+
| 1 key       | 240         | 88            |
+-------------+-------------+---------------+
| 2 key       | 240         | 104           |
+-------------+-------------+---------------+
| 100 key     | 4704        | 2120          |
+-------------+-------------+---------------+

|sortedmap| is not meant to replace dictionaries but can be valuable
when a large number of small containers is necessary.
