# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Tools to declare static observers in Atom subclasses"""
from types import FunctionType
from typing import (
    TYPE_CHECKING,
    Any,
    Callable,
    List,
    Mapping,
    Optional,
    Tuple,
    TypeVar,
    Union,
)

from ..catom import ChangeType
from ..typing_utils import ChangeDict

if TYPE_CHECKING:
    from ..atom import Atom


def observe(*names: str, change_types: ChangeType = ChangeType.ANY) -> "ObserveHandler":
    """A decorator which can be used to observe members on a class.

    Parameters
    ----------
    *names
        The str names of the attributes to observe on the object.
        These must be of the form 'foo' or 'foo.bar'.
    change_types
        The flag specifying the type of changes to observe.

    """
    # backwards compatibility for a single tuple or list argument
    if len(names) == 1 and isinstance(names[0], (tuple, list)):
        names = names[0]
    pairs: List[Tuple[str, Optional[str]]] = []
    for name in names:
        if not isinstance(name, str):
            msg = "observe attribute name must be a string, got '%s' instead"
            raise TypeError(msg % type(name).__name__)
        ndots = name.count(".")
        if ndots > 1:
            msg = "cannot observe '%s', only a single extension is allowed"
            raise TypeError(msg % name)
        if ndots == 1:
            name, attr = name.split(".")
            pairs.append((name, attr))
        else:
            pairs.append((name, None))
    return ObserveHandler(pairs, change_types)


T = TypeVar("T", bound="Atom")


class ObserveHandler(object):
    """An object used to temporarily store observe decorator state."""

    __slots__ = ("pairs", "func", "funcname", "change_types")

    #: List of 2-tuples which stores the pair information for the observers.
    pairs: List[Tuple[str, Optional[str]]]

    #: Callable to be used as observer callback.
    func: Optional[Callable[[Mapping[str, Any]], None]]

    #: Name of the callable. Used by the metaclass.
    funcname: Optional[str]

    #: Types of changes to listen to.
    change_types: ChangeType

    def __init__(
        self,
        pairs: List[Tuple[str, Optional[str]]],
        change_types: ChangeType = ChangeType.ANY,
    ) -> None:
        """Initialize an ObserveHandler.

        Parameters
        ----------
        pairs : list
            The list of 2-tuples which stores the pair information for the observers.

        """
        self.pairs = pairs
        self.change_types = change_types
        self.func = None  # set by the __call__ method
        self.funcname = None

    def __call__(
        self,
        func: Union[
            Callable[[ChangeDict], None],
            Callable[[T, ChangeDict], None],
            # AtomMeta will replace ObserveHandler in the body of an atom
            # class allowing to access it for example in a subclass. We lie here by
            # giving ObserverHandler.__call__ a signature compatible with an
            # observer to mimic this behavior.
            ChangeDict,
        ],
    ) -> "ObserveHandler":
        """Called to decorate the function.

        Parameters
        ----------
        func
            Should be either a callable taking as single argument the change
            dictionary or a method declared on an Atom object.

        """
        assert isinstance(func, FunctionType), "func must be a function"
        self.func = func
        return self

    def clone(self) -> "ObserveHandler":
        """Create a clone of the sentinel."""
        clone = type(self)(self.pairs, self.change_types)
        clone.func = self.func
        return clone


class ExtendedObserver(object):
    """A callable object used to implement extended observers."""

    __slots__ = ("funcname", "attr")

    #: Name of the function on the owner object which should be used as the observer.
    funcname: str

    #: Attribute name on the target object which should be observed.
    attr: str

    def __init__(self, funcname: str, attr: str) -> None:
        """Initialize an ExtendedObserver.

        Parameters
        ----------
        funcname : str
            The function name on the owner object which should be
            used as the observer.

        attr : str
            The attribute name on the target object which should be
            observed.

        """
        self.funcname = funcname
        self.attr = attr

    def __call__(self, change: ChangeDict) -> None:
        """Handle a change of the target object.

        This handler will remove the old observer and attach a new
        observer to the target attribute. If the target object is not
        an Atom object, an exception will be raised.

        """
        from ..atom import Atom

        old = None
        new = None
        ctype = change["type"]
        if ctype == "create":
            new = change["value"]
        elif ctype == "update":
            old = change["oldvalue"]
            new = change["value"]
        elif ctype == "delete":
            old = change["value"]
        attr = self.attr
        owner = change["object"]
        handler = getattr(owner, self.funcname)
        if isinstance(old, Atom):
            old.unobserve(attr, handler)
        if isinstance(new, Atom):
            new.observe(attr, handler)
        elif new is not None:
            msg = "cannot attach observer '%s' to non-Atom %s"
            raise TypeError(msg % (attr, new))
