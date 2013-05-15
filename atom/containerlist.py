#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate
from .list import List#, ListProxy
from .instance import Instance
from .typed import Typed


class ContainerList(List):
    """ A List member which supports container notifications.

    """
    __slots__ = ()

    def __init__(self, item=None, default=None):
        """ Initialize a ContainerList.

        """
        if item is not None and not isinstance(item, Member):
            if isinstance(item, type):
                item = Typed(item)
            else:
                item = Instance(item)
        self.item = item
        self.set_default_value_mode(DefaultValue.List, default)
        self.set_validate_mode(Validate.ContainerList, item)


class ContainerListProxy(list):#ListProxy):
    """ A proxy which validates and notifies in-place list operations.

    Instances of this class are created on the fly by the post getattr
    handler of a ContainerList.

    """
    # TODO move this class to C++
    __slots__ = ()

    def __iadd__(self, items):
        member = self._member
        validator = member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            items = [validate(owner, None, i) for i in items]
        owner = self._owner
        obss = member.has_observers()
        obsd = owner.has_observers(member.name)
        if obss or obsd:
            if not isinstance(items, list):
                items = list(items)
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': '__iadd__',
                'items': items,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
        else:
            self._value += items
        return self._value

    def __imul__(self, count):
        self._value *= count
        owner = self._owner
        member = self._member
        obss = member.has_observers()
        obsd = owner.has_observers(member.name)
        if obss or obsd:
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': '__imul__',
                'count': count,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
        return self._value

    def __setitem__(self, index, item):
        member = self._member
        validator = member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            if isinstance(index, slice):
                item = [validate(owner, None, i) for i in item]
            else:
                item = validate(owner, None, item)
        owner = self._owner
        obss = member.has_observers()
        obsd = owner.has_observers(member.name)
        if obss or obsd:
            if isinstance(index, slice) and not isinstance(item, list):
                item = list(item)
            olditem = self._value[index]
            self._value[index] = item
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': '__setitem__',
                'index': index,
                'olditem': olditem,
                'newitem': item,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
        else:
            self._value[index] = item

    def __delitem__(self, index):
        owner = self._owner
        member = self._member
        obss = member.has_observers()
        obsd = owner.has_observers(member.name)
        if obss or obsd:
            listitem = self._value[index]
            del self._value[index]
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': '__delitem__',
                'index': index,
                'item': listitem,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
        else:
            del self._value[index]
