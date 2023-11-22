# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from contextlib import contextmanager
from typing import ClassVar, Iterator, Mapping

from .catom import CAtom, Member
from .meta import AtomMeta, add_member, observe, set_default  # noqa

# The above are imported to avoid breaking code relying on internals


def __newobj__(cls, *args):
    """A compatibility pickler function.

    This function is not part of the public Atom api.

    """
    return cls.__new__(cls, *args)


class Atom(CAtom, metaclass=AtomMeta):
    """The base class for defining atom objects.

    `Atom` objects are special Python objects which never allocate an
    instance dictionary unless one is explicitly requested. The storage
    for an atom is instead computed from the `Member` objects declared
    on the class. Memory is reserved for these members with no over
    allocation.

    This restriction make atom objects a bit less flexible than normal
    Python objects, but they are between 3x-10x more memory efficient
    than normal objects depending on the number of attributes.

    """

    __atom_members__: ClassVar[Mapping[str, Member]]

    @classmethod
    def members(cls) -> Mapping[str, Member]:
        """Get the members dictionary for the type.

        Returns
        -------
        result : Mapping[str, Member]
            The dictionary of members defined on the class. User code
            should not modify the contents of the dict.

        """
        return cls.__atom_members__

    @contextmanager
    def suppress_notifications(self) -> Iterator[None]:
        """Disable member notifications within in a context.

        Returns
        -------
        result : contextmanager
            A context manager which disables atom notifications for the
            duration of the context. When the context exits, the state
            is restored to its previous value.

        """
        old = self.set_notifications_enabled(False)
        yield
        self.set_notifications_enabled(old)

    def __reduce_ex__(self, proto):
        """An implementation of the reduce protocol.

        This method creates a reduction tuple for Atom instances. This
        method should not be overridden by subclasses unless the author
        fully understands the rammifications.

        """
        args = (type(self), *self.__getnewargs__())
        return (__newobj__, args, self.__getstate__())

    def __getnewargs__(self):
        """Get the argument tuple to pass to __new__ on unpickling.

        See the Python.org docs for more information.

        """
        return ()
