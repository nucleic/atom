#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate


class Coerced(Member):
    """ A member which will coerce a value to a given type.

    """
    __slots__ = ('kind', 'coercer')

    def __init__(self, kind, factory=None, coercer=None):
        """ Initialize a Coerced.

        Parameters
        ----------
        kind : type or tuple of types
            The allowable types for the value.

        factory : callable, optional
            An optional callable which takes no arguments and returns
            the default value for the member. If this is not provided
            the default value will be None.

        coercer : callable, optional
            An optional callable which takes the value and returns the
            coerced value. If this is not given, then 'kind' must be
            a callable type which will be called to coerce the value.

        """
        self.kind = kind
        self.coercer = coercer
        if factory is not None:
            self.set_default_kind(DefaultValue.CallObject, factory)
        else:
            self.set_default_kind(DefaultValue.Static, None)
        self.set_validate_kind(Validate.MemberMethod_ObjectOldNew, 'validate')

    def validate(self, owner, old, new):
        """ Validate the value of the member.

        If the value is not an instance of the allowable types, it will
        be coerced to a value of the appropriate type.

        """
        kind = self.kind
        if isinstance(new, kind):
            return new
        coercer = self.coercer or kind
        try:
            res = coercer(new)
        except (TypeError, ValueError):
            raise TypeError('could not coerce value an appopriate type')
        return res
