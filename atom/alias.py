#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------


class Alias(object):
    """ An Atom attribute whose value is Aliased to another Atom Instance

    The value is read from the other member and written to the other
    member.
    
    """
    __slots__ = ('other', 'alias_name', 'attr_name')
    
    def __init__(self, other, alias_name=None):
        # other starts out as a reference to the other object
        self.other = other
        self.alias_name = alias_name
        self.attr_name = None
        
    def __get__(self, instance, owner):
        attr = self.alias_name or self.attr_name
        # throw out our reference to the other Atom and just use its name
        if not isinstance(self.other, str):
            self.other = self.other.name
        obj = getattr(instance, self.other)
        return getattr(obj, attr)
    
    def __set__(self, instance, value):
        attr = self.alias_name or self.attr_name
        # throw out our reference to the other Atom and just use its name
        if not isinstance(self.other, str):
            self.other = self.other.name
        obj = getattr(instance, self.other)
        setattr(obj, attr, value)
