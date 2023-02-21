# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import (
    DefaultValue,
    Member,
    PostGetAttr,
    PostSetAttr,
    PostValidate,
    Validate,
)


class Delegator(Member):
    """A member subclass which delegates all work to a wrapped member.

    The only behaviors not delegated are GetAttr and SetAttr. Subclasses
    should override behavior as needed to suit their needs. In order to
    change a particular mode, the relevant change method must be called
    via super(Delegator, ...).

    """

    __slots__ = "delegate"

    def __init__(self, delegate):
        """Initialize a DeclarativeProperty.

        Parameters
        ----------
        delegate : Member
            The Atom Member which provides the behavior for the property.
            The member should use standard slot behavior semantics.

        """
        self.delegate = delegate
        sup = super(Delegator, self)
        sup.set_post_getattr_mode(PostGetAttr.Delegate, delegate)
        sup.set_post_setattr_mode(PostSetAttr.Delegate, delegate)
        sup.set_default_value_mode(DefaultValue.Delegate, delegate)
        sup.set_validate_mode(Validate.Delegate, delegate)
        sup.set_post_validate_mode(PostValidate.Delegate, delegate)

    def add_static_observer(self, observer):
        """Add a static observer to the member.

        This method also adds the static observer to the delegate.

        """
        super(Delegator, self).add_static_observer(observer)
        self.delegate.add_static_observer(observer)

    def remove_static_observer(self, observer):
        """Remove a static observer from the member.

        This method also removes the static observer from the delegate.

        """
        super(Delegator, self).remove_static_observer(observer)
        self.delegate.remove_static_observer(observer)

    def set_name(self, name):
        """Assign the name to this member.

        This method keeps the name of the delegate member in sync.

        """
        super(Delegator, self).set_name(name)
        self.delegate.set_name(name)

    def set_index(self, index):
        """Assign the index to this member.

        This method keeps the index of the delegate member in sync.

        """
        super(Delegator, self).set_index(index)
        self.delegate.set_index(index)

    def set_post_getattr_mode(self, mode, context):
        """Set the post getattr mode for the member.

        This method proxies the change to the delegate member.

        """
        self.delegate.set_post_getattr_mode(mode, context)

    def set_post_setattr_mode(self, mode, context):
        """Set the post getattr mode for the member.

        This method proxies the change to the delegate member.

        """
        self.delegate.set_post_setattr_mode(mode, context)

    def set_default_value_mode(self, mode, context):
        """Set the default value mode for the member.

        This method proxies the change to the delegate member.

        """
        self.delegate.set_default_value_mode(mode, context)

    def set_validate_mode(self, mode, context):
        """Set the default value mode for the member.

        This method proxies the change to the delegate member.

        """
        self.delegate.set_validate_mode(mode, context)

    def set_post_validate_mode(self, mode, context):
        """Set the default value mode for the member.

        This method proxies the change to the delegate member.

        """
        self.delegate.set_post_validate_mode(mode, context)

    def clone(self):
        """Create a clone of the declarative property.

        This method also creates a clone of the internal delegate for
        mode handlers which use the original delegate as the context.

        """
        clone = super(Delegator, self).clone()
        delegate = self.delegate
        clone.delegate = delegate_clone = delegate.clone()

        mode, old = clone.post_getattr_mode
        if old is delegate:
            clone.set_post_getattr_mode(mode, delegate_clone)

        mode, old = clone.post_setattr_mode
        if old is delegate:
            clone.set_post_setattr_mode(mode, delegate_clone)

        mode, old = clone.default_value_mode
        if old is delegate:
            clone.set_default_value_mode(mode, delegate_clone)

        mode, old = clone.validate_mode
        if old is delegate:
            clone.set_validate_mode(mode, delegate_clone)

        mode, old = clone.post_validate_mode
        if old is delegate:
            clone.set_post_validate_mode(mode, delegate_clone)

        return clone
