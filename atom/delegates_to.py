#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, PostSetAttr, PostGetAttr


class DelegatesTo(Member):
    """ A Member whose value is Delegated to another Atom

    The value is read from the other member and written to the other
    member.

    """
    __slots__ = ('other', 'alias', 'parent')

    def __init__(self, other, alias=None):
        """ Initialize a DelegatesTo.

        Parameters
        ----------
        other : Member
            The other member to sync the value with.

        alias : str, optional
            The attribute name on the target object which should be
            synced, if different.

        """
        super(DelegatesTo, self).__init__()
        self.other = other
        self.alias = alias
        self.parent = None
        mode = PostSetAttr.MemberMethod_ObjectOldNew
        self.set_post_setattr_mode(mode, 'post_setattr')
        mode = PostGetAttr.MemberMethod_ObjectValue
        self.set_post_getattr_mode(mode, 'post_getattr')
        self.other.add_static_observer(self.sync_obj)

    def post_setattr(self, owner, old, new):
        '''Send the value to the other member
        '''
        if not self.alias:
            self.alias = self.name
        obj = getattr(owner, self.other.name)
        setattr(obj, self.alias, new)

    def post_getattr(self, owner, value):
        if not self.alias:
            self.alias = self.name
        obj = getattr(owner, self.other.name)
        return getattr(obj, self.alias)

    def sync_obj(self, change):
        '''Set up observing changes to the other member attribute.
        '''
        # We now have a reference to our parent object and the other object
        if not self.alias:
            self.alias = self.name
        obj = change['value']
        mem = obj.get_member(self.alias)
        mem.add_static_observer(self.sync_value)
        self.parent = change['object']
        self.other.remove_static_observer(self.sync_obj)
        value = getattr(change['value'], self.alias)
        if value:
            setattr(self.parent, self.name, value)
        else:
            value = getattr(self.parent, self.name)
            setattr(change['value'], self.alias, value)

    def sync_value(self, change):
        '''Forward changes from the other member
        '''
        setattr(self.parent, self.name, change['value'])
