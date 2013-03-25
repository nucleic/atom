#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import PostGetAttr
from .list import List, ListProxy


class ContainerList(List):
    """ A List member which supports container notifications.

    """
    __slots__ = ()

    def __init__(self, item=None, default=None, copy=True):
        """ Initialize a ContainerList.

        """
        super(ContainerList, self).__init__(item, default, copy)
        self.set_post_getattr_mode(
            PostGetAttr.MemberMethod_ObjectValue, "post_getattr"
        )

    def post_getattr(self, owner, value):
        """ A post getattr handler.

        This handler is only invoked if the list uses an item validator.

        """
        return ContainerListProxy(self, owner, value)


class ContainerListProxy(ListProxy):
    """ A proxy which validates and notifies in-place list operations.

    Instances of this class are created on the fly by the post getattr
    handler of a ContainerList.

    """
    # TODO move this class to C++
    __slots__ = ()

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

    def append(self, item):
        member = self._member
        validator = member.item
        if validator is not None:
            validate = validator.do_full_validate
            item = validate(self._owner, None, item)
        self._value.append(item)
        owner = self._owner
        obss = member.has_observers()
        obsd = owner.has_observers(member.name)
        if obss or obsd:
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': 'append',
                'item': item,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)

    def extend(self, items):
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
            self._value.extend(items)
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': 'extend',
                'items': items,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
        else:
            self._value.extend(items)

    def insert(self, index, item):
        member = self._member
        validator = member.item
        if validator is not None:
            validate = validator.do_full_validate
            item = validate(self._owner, None, item)
        self._value.insert(index, item)
        owner = self._owner
        obss = member.has_observers()
        obsd = owner.has_observers(member.name)
        if obss or obsd:
            change = {
                'type': 'container',
                'name': member.name,
                'object': owner,
                'value': self._value,
                'operation': 'insert',
                'index': index,
                'item': item,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)

    def pop(self, *args):
        item = self._value.pop(*args)
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
                'operation': 'pop',
                'index': args[0] if args else None,
                'item': item,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
        return item

    def remove(self, item):
        self._value.remove(item)
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
                'operation': 'remove',
                'item': item,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)

    def reverse(self):
        self._value.reverse()
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
                'operation': 'reverse',
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)

    def sort(self, cmp=None, key=None, reverse=False):
        self._value.sort(cmp, key, reverse)
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
                'operation': 'sort',
                'cmp': cmp,
                'key': key,
                'reverse': reverse,
            }
            if obss:
                member.notify(owner, change)
            if obsd:
                owner.notify(member.name, change)
