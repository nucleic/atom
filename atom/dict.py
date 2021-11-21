# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import DefaultValue, Member, Validate
from .instance import Instance
from .typing_utils import extract_types, is_optional


class Dict(Member):
    """A value of type `dict`."""

    __slots__ = ()

    def __init__(self, key=None, value=None, default=None):
        """Initialize a Dict.

        Parameters
        ----------
        key : Member, type, tuple of types, or None, optional
            A member to use for validating the types of keys allowed in
            the dict. This can also be a type or a tuple of types, which
            will be wrapped with an Instance member. If this is not
            given, no key validation is performed.

        value : Member, type, tuple of types, or None, optional
            A member to use for validating the types of values allowed
            in the dict. This can also be a type or a tuple of types,
            which will be wrapped with an Instance member. If this is
            not given, no value validation is performed.

        default : dict, optional
            The default dict of items. A new copy of this dict will be
            created for each atom instance.

        """
        self.set_default_value_mode(DefaultValue.Dict, default)
        if key is not None and not isinstance(key, Member):
            opt, types = is_optional(extract_types(key))
            key = Instance(types, optional=opt)
        if value is not None and not isinstance(value, Member):
            opt, types = is_optional(extract_types(value))
            value = Instance(types, optional=opt)
        self.set_validate_mode(Validate.Dict, (key, value))

    def set_name(self, name):
        """Assign the name to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the name of the internal members are
        also updated.

        """
        super(Dict, self).set_name(name)
        key, value = self.validate_mode[1]
        if key is not None:
            key.set_name(name + "|key")
        if value is not None:
            value.set_name(name + "|value")

    def set_index(self, index):
        """Assign the index to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the index of the internal members are
        also updated.

        """
        super(Dict, self).set_index(index)
        key, value = self.validate_mode[1]
        if key is not None:
            key.set_index(index)
        if value is not None:
            value.set_index(index)

    def clone(self):
        """Create a clone of the member.

        This will clone the internal dict key and value members if they exist.

        """
        clone = super(Dict, self).clone()
        key, value = self.validate_mode[1]
        if key is not None or value is not None:
            key_clone = key.clone() if key is not None else None
            value_clone = value.clone() if value is not None else None
            mode, _ = self.validate_mode
            clone.set_validate_mode(mode, (key_clone, value_clone))
        return clone
