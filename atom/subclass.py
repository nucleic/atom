#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, print_function, absolute_import)

from .catom import Member, DefaultValue, Validate


class Subclass(Member):
    """ A value which allows objects subtypes of a given type.

    Values will be tested using the `PyObject_IsSubclass` C API call.
    This call is equivalent to `issubclass(value, kind)` and all the
    same rules apply.

    A Subclass member cannot be set to None.

    """
    __slots__ = ()

    def __init__(self, kind, default=None):
        """ Initialize a Subclass member.

        Parameters
        ----------
        kind : type or tuple of types
            The allowed type or types for the subclass. This will be
            used as the default value if no default is given.

        default : type, optional
            The default value for the member. If this is not provided,
            'kind' will be used as the default.

        """
        self.set_default_value_mode(DefaultValue.Static, default or kind)
        self.set_validate_mode(Validate.Subclass, kind)


class ForwardSubclass(Subclass):
    """ A Subclass which delays resolving the type definition.

    The first time the value is accessed or modified, the type will
    be resolved and the forward subclass will behave identically to
    a normal subclass.

    """
    __slots__ = 'resolve'

    def __init__(self, resolve):
        """ Initialize a ForwardSubclass member.

        resolve : callable
            A callable which takes no arguments and returns the type or
            tuple of types to use for validating the subclass values.

        """
        self.resolve = resolve
        self.set_default_value_mode(DefaultValue.MemberMethod_Object, "default")
        self.set_validate_mode(Validate.MemberMethod_ObjectOldNew, "validate")

    def default(self, owner):
        """ Called to retrieve the default value.

        This is called the first time the default value is retrieved
        for the member. It resolves the type and updates the internal
        default handler to behave like a normal Subclass member.

        """
        kind = self.resolve()
        self.set_default_value_mode(DefaultValue.Static, kind)
        return kind

    def validate(self, owner, old, new):
        """ Called to validate the value.

        This is called the first time a value is validated for the
        member. It resolves the type and updates the internal validate
        handler to behave like a normal Subclass member.

        """
        kind = self.resolve()
        self.set_validate_mode(Validate.Subclass, kind)
        return self.do_validate(owner, old, new)

    def clone(self):
        """ Create a clone of the ForwardSubclass object.

        """
        clone = super(ForwardSubclass, self).clone()
        clone.resolve = self.resolve
        return clone
