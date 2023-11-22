# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Metaclass implementing atom members customization."""
import copyreg
import warnings
from types import FunctionType
from typing import (
    Any,
    Callable,
    Dict,
    FrozenSet,
    List,
    Mapping,
    MutableMapping,
    Optional,
    Sequence,
    Set,
    Tuple,
    TypeVar,
    Union,
)

from ..catom import (
    CAtom,
    DefaultValue,
    GetState,
    Member,
    PostGetAttr,
    PostSetAttr,
    PostValidate,
    Validate,
)
from .annotation_utils import generate_members_from_cls_namespace
from .member_modifiers import set_default
from .observation import ExtendedObserver, ObserveHandler

OBSERVE_PREFIX = "_observe_"
DEFAULT_PREFIX = "_default_"
VALIDATE_PREFIX = "_validate_"
POST_GETATTR_PREFIX = "_post_getattr_"
POST_SETATTR_PREFIX = "_post_setattr_"
POST_VALIDATE_PREFIX = "_post_validate_"
GETSTATE_PREFIX = "_getstate_"


M = TypeVar("M", bound=Member)


def add_member(cls: "AtomMeta", name: str, member: Member) -> None:
    """Add or override a member after the class creation."""
    existing = cls.__atom_members__.get(name)
    if existing is not None:
        member.set_index(existing.index)
        member.copy_static_observers(existing)
    else:
        member.set_index(len(cls.__atom_members__))

    member.set_name(name)
    # The dict is mutable but we do not want to say it too loud
    cls.__atom_members__[name] = member  # type: ignore
    cls.__atom_specific_members__ = frozenset(
        set(cls.__atom_specific_members__) | {name}
    )
    setattr(cls, name, member)


def clone_if_needed(cls: "AtomMeta", member: M) -> M:
    """Clone a member if is not owned by a class.

    This function is meant to be used in __init__subclass__ to safely
    customize members static behaviors.

    """
    members = dict(cls.__atom_members__)
    specific_members = set(cls.__atom_specific_members__)
    # This may lead to cloning some members that do not need to be but it
    # should not be too costly
    owned_members = {members[k] for k in cls.__atom_specific_members__}
    m = _clone_if_needed(member, members, specific_members, owned_members)
    setattr(cls, m.name, m)
    cls.__atom_members__ = members
    cls.__atom_specific_members__ = frozenset(specific_members)
    return m


class MissingMemberWarning(UserWarning):
    """Signal an expected member is not present."""

    pass


def _signal_missing_member(
    owner: str, method: str, members: Mapping[str, Member], prefix: str
) -> None:
    warnings.warn(
        f"{prefix} method {method} on class '{owner}' does not match any member "
        f"defined on the Atom object. Existing members are: {members}",
        MissingMemberWarning,
        stacklevel=3,
    )


def _compute_mro(bases: Sequence[type]) -> List[type]:
    """Compute the MRO from a sequence of base classes using the C3 algorithm.

    Adapted from https://www.python.org/download/releases/2.3/mro/.

    """
    sequences = [b.mro() for b in bases]
    computed_mro: List[type] = []

    while True:
        sequences = list(filter(None, sequences))
        if not sequences:
            return computed_mro

        # Look for candidates in seq heads
        candidate: Optional[type]
        for s1 in sequences:
            candidate = s1[0]
            # Discard candidate if it appears in the tail of any of the sequence
            if any(candidate in s[1:] for s in sequences):
                candidate = None
            else:
                break
        if candidate is None:
            raise RuntimeError("Inconsistent hierarchy")
        computed_mro.append(candidate)

        # Remove the candidate from all sequences
        for seq in sequences:
            if seq[0] is candidate:
                del seq[0]


def _clone_if_needed(
    member: M,
    members: Dict[str, Member],
    specific_members: Set[str],
    owned_members: Set[Member],
) -> M:
    """Clone a member if it cannot be safely modified."""
    # The member may have been cloned due to slot conflicts but that
    # does not make it specific. However, each member on which function
    # is called is guaranteed to be specific.
    specific_members.add(member.name)
    if member not in owned_members:
        member = member.clone()
        members[member.name] = member
        owned_members.add(member)
    return member


class _AtomMetaHelper:
    """Helper class to create a Atom class."""

    #: All members that can be accessed from the class we are helping create.
    members: MutableMapping[str, Member]

    #: The set of members which live on this class as opposed to a
    #: base class. This enables the code which hooks up the various
    #: static behaviors to only clone a member when necessary.
    owned_members: Set[Member]

    #: Names of the members whose behavior is specific to this class.
    specific_members: Set[str]

    #: Decorated observers: @observe
    decorated: List[ObserveHandler]

    #: The set of seen @observe decorators
    seen_decorated: Set[ObserveHandler]

    #: set_default() sentinel
    set_defaults: List[set_default]

    #: Static observer methods: _observe_*
    observes: List[str]

    #: Default value methods: _default_*
    defaults: List[str]

    #: Validator methods: _validate_*
    validates: List[str]

    #: Post getattr methods: _post_getattr_*
    post_getattrs: List[str]

    #: Post setattr methods: _post_setattr_*
    post_setattrs: List[str]

    #: Post validate methods: _post_validate_*
    post_validates: List[str]

    # Getstate methods: _getstate_*
    getstates: List[str]

    __slots__ = (
        "name",
        "bases",
        "dct",
        "members",
        "owned_members",
        "specific_members",
        "observes",
        "defaults",
        "validates",
        "decorated",
        "set_defaults",
        "post_getattrs",
        "post_setattrs",
        "post_validates",
        "getstates",
        "seen_decorated",
    )

    def __init__(self, name: str, bases: Tuple[type, ...], dct: Dict[str, Any]) -> None:
        self.name = name
        self.bases = bases
        self.dct = dct
        self.members = {}
        self.owned_members = set()
        self.specific_members = set()
        self.observes = []
        self.defaults = []
        self.validates = []
        self.decorated = []
        self.set_defaults = []
        self.post_getattrs = []
        self.post_setattrs = []
        self.post_validates = []
        self.getstates = []
        self.seen_decorated = set()

    def scan_and_clear_namespace(self) -> None:
        """Collect information necessary to implement the various behaviors.

        Some objects declared on the class only serve as sentinels, and they are
        removed from the dict before creating the class.

        """
        dct = self.dct
        seen_sentinels = set()  # The set of seen sentinels
        for key, value in dct.items():
            if isinstance(value, set_default):
                if value in seen_sentinels:
                    value = value.clone()
                value.name = key
                self.set_defaults.append(value)
                seen_sentinels.add(value)
                continue
            if isinstance(value, ObserveHandler):
                if value in self.seen_decorated:
                    value = value.clone()
                self.seen_decorated.add(value)
                self.decorated.append(value)
                value.funcname = key
                value = value.func
                dct[key] = value
                # Coninue processing the unwrapped function
            if isinstance(value, FunctionType):
                if key.startswith(OBSERVE_PREFIX):
                    self.observes.append(key)
                elif key.startswith(DEFAULT_PREFIX):
                    self.defaults.append(key)
                elif key.startswith(VALIDATE_PREFIX):
                    self.validates.append(key)
                elif key.startswith(POST_VALIDATE_PREFIX):
                    self.post_validates.append(key)
                elif key.startswith(POST_GETATTR_PREFIX):
                    self.post_getattrs.append(key)
                elif key.startswith(POST_SETATTR_PREFIX):
                    self.post_setattrs.append(key)
                elif key.startswith(GETSTATE_PREFIX):
                    self.getstates.append(key)

        # Remove the sentinels from the dict before creating the class.
        # The sentinels for the @observe decorators are already removed.
        for s in seen_sentinels:
            assert s.name
            del dct[s.name]

    def assign_members_indexes(self) -> None:
        """Walk the MRO to assign a unique index to each member."""
        # Compute the class MRO.
        cls_mro = _compute_mro(self.bases)

        # Walk the mro of the class, excluding itself, in reverse order
        # collecting all of the members into a single dict. The reverse
        # update preserves the mro of overridden members. We use only known
        # specific members to also preserve the mro in presence of multiple
        # inheritance.
        members = self.members
        for base in reversed(cls_mro[:-1]):
            if base is not CAtom and issubclass(base, CAtom):
                # Except if somebody abuses the system and create a non-Atom subclass
                # of CAtom, this works
                # Mypy does not narrow the type from the above test hence the ignores
                members.update(
                    {
                        k: v
                        for k, v in base.__atom_members__.items()  # type: ignore
                        if k in base.__atom_specific_members__  # type: ignore
                    }
                )

        # Resolve any conflicts with memory layout. Conflicts can occur
        # with multiple inheritance where the indices of multiple base
        # classes will overlap. When this happens, the members which
        # conflict must be cloned in order to occupy a unique index.
        conflicts = []
        occupied = set()
        for member in members.values():
            if member.index in occupied:
                conflicts.append(member)
            else:
                occupied.add(member.index)

        # Track the first available index
        i = 0

        def get_first_free_index() -> int:
            nonlocal i
            while i in occupied:
                i += 1
            occupied.add(i)
            return i

        # Clone the conflicting members and give them a unique index.
        # Do not blow away an overridden item on the current class.
        owned_members = self.owned_members
        cloned = {}
        for member in conflicts:
            clone = member.clone()
            clone.set_index(get_first_free_index())
            owned_members.add(clone)
            members[clone.name] = clone
            if clone.name not in self.dct:
                cloned[clone.name] = clone

        # Walk the dict a second time to collect the class members. This
        # assigns the name and the index to the member. If a member is
        # overriding an existing member, the memory index of the old
        # member is reused and any static observers are copied over.
        specific_members = self.specific_members
        for key, value in self.dct.items():
            if isinstance(value, Member):
                if value in owned_members:  # foo = bar = Baz()
                    value = value.clone()
                    self.dct[key] = value

                value.set_name(key)
                owned_members.add(value)
                specific_members.add(value.name)
                if key in members:
                    supermember = members[key]
                    members[key] = value
                    value.set_index(supermember.index)
                    value.copy_static_observers(supermember)
                else:
                    value.set_index(get_first_free_index())
                    members[key] = value

        # Ensure we have a contiguous array for members
        assert occupied == set(range(len(members)))

        # Add cloned members to the class dictionary
        self.dct.update(cloned)

    def apply_members_static_behaviors(self) -> None:
        """Add the special statically defined behaviors for the members.

        If the target member is defined on a subclass, it is cloned
        so that the behavior of the subclass is not modified.

        """
        members = self.members

        def clone_if_needed(m):
            m = _clone_if_needed(m, members, self.specific_members, self.owned_members)
            self.dct[m.name] = m
            return m

        # set_default() sentinels
        for sd in self.set_defaults:
            assert sd.name  # At this point the name has been set
            if sd.name not in members:
                msg = "Invalid call to set_default(). '%s' is not a member "
                msg += "on the '%s' class."
                raise TypeError(msg % (sd.name, self.name))
            member = clone_if_needed(members[sd.name])
            member.set_default_value_mode(DefaultValue.Static, sd.value)

        # _default_* methods
        for prefix, method_names, mode_setter in [
            (
                DEFAULT_PREFIX,
                self.defaults,
                lambda m, name: m.set_default_value_mode(
                    DefaultValue.ObjectMethod, name
                ),
            ),
            (
                VALIDATE_PREFIX,
                self.validates,
                lambda m, name: m.set_validate_mode(Validate.ObjectMethod_OldNew, name),
            ),
            (
                POST_VALIDATE_PREFIX,
                self.post_validates,
                lambda m, name: m.set_post_validate_mode(
                    PostValidate.ObjectMethod_OldNew, name
                ),
            ),
            (
                POST_GETATTR_PREFIX,
                self.post_getattrs,
                lambda m, name: m.set_post_getattr_mode(
                    PostGetAttr.ObjectMethod_Value, name
                ),
            ),
            (
                POST_SETATTR_PREFIX,
                self.post_setattrs,
                lambda m, name: m.set_post_setattr_mode(
                    PostSetAttr.ObjectMethod_OldNew, name
                ),
            ),
            (
                GETSTATE_PREFIX,
                self.getstates,
                lambda m, name: m.set_getstate_mode(GetState.ObjectMethod_Name, name),
            ),
        ]:
            n = len(prefix)
            for mangled in method_names:
                target = mangled[n:]
                if target in members:
                    member = clone_if_needed(members[target])
                    mode_setter(member, mangled)
                else:
                    _signal_missing_member(self.name, mangled, members, prefix)

        # _observe_* methods
        n = len(OBSERVE_PREFIX)
        for mangled in self.observes:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.add_static_observer(mangled)
            else:
                _signal_missing_member(self.name, mangled, members, OBSERVE_PREFIX)

        # @observe decorated methods
        for handler in self.decorated:
            assert handler.funcname  # Set at this point
            change_types = handler.change_types
            for name, attr in handler.pairs:
                if name in members:
                    member = clone_if_needed(members[name])
                    observer: Union[str, Callable[..., None]]
                    observer = handler.funcname
                    if attr is not None:
                        observer = ExtendedObserver(observer, attr)
                    member.add_static_observer(observer, change_types)
                else:
                    _signal_missing_member(
                        self.name, name, members, "observe decorated"
                    )

    def create_class(self, meta: type) -> type:
        """Create the class after adding class variables."""

        # Put a reference to the members dict on the class. This is used
        # by CAtom to query for the members and member count as needed.
        self.dct["__atom_members__"] = self.members

        # Keep a reference to the specific members dict on the class. Specific
        # members are members which are defined or altered in the class. This
        # is used to ensure proper MRO resolution for members.
        self.dct["__atom_specific_members__"] = frozenset(
            m for m in self.specific_members
        )

        # Create the class object.
        # We do it once everything else has been setup so that if users wants
        # to use __init__subclass__ they have access to fully initialized
        # Atom type.
        cls: type = type.__new__(meta, self.name, self.bases, self.dct)

        # Generate slotnames cache
        # (using a private function that mypy does not know about).
        copyreg._slotnames(cls)  # type: ignore

        return cls


class AtomMeta(type):
    """The metaclass for classes derived from Atom.

    This metaclass computes the memory layout of the members in a given
    class so that the CAtom class can allocate exactly enough space for
    the object data slots when it instantiates an object.

    All classes deriving from Atom will be automatically slotted, which
    will prevent the creation of an instance dictionary and also the
    ability of an Atom to be weakly referenceable. If that behavior is
    required, then a subclasss should declare the appropriate slots.

    """

    __atom_members__: Mapping[str, Member]
    __atom_specific_members__: FrozenSet[str]

    def __new__(
        meta,
        name: str,
        bases: Tuple[type, ...],
        dct: Dict[str, Any],
        enable_weakrefs: bool = False,
        use_annotations: bool = True,
        type_containers: int = 1,
    ):
        # Ensure there is no weird mro calculation and that we can use our
        # re-implementation of C3
        assert meta.mro is type.mro, "Custom MRO calculation are not supported"

        # Unless the developer requests slots, they are automatically
        # turned off. This prevents the creation of instance dicts and
        # other space consuming features unless explicitly requested.
        if "__slots__" not in dct:
            dct["__slots__"] = ()
        if enable_weakrefs:
            dct["__slots__"] += ("__weakref__",)

        if use_annotations and "__annotations__" in dct:
            generate_members_from_cls_namespace(name, dct, type_containers)

        # Create the helper used to analyze the namespace and customize members
        helper = _AtomMetaHelper(name, bases, dct)

        # Analyze and clean the namespace
        helper.scan_and_clear_namespace()

        # Assign each member a unique ID
        helper.assign_members_indexes()

        # Customize the members based on the specified static modifiers
        helper.apply_members_static_behaviors()

        return helper.create_class(meta)
