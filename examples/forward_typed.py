#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from atom.api import Atom, Unicode, ForwardTyped, Instance


class Dog(Atom):
    
    first_name = Unicode()
    
    last_name = Unicode()
    
    owner = ForwardTyped(lambda: Person)
    
        
class Person(Atom):
    """ A simple class representing a person object.

    """
    first_name = Unicode()
    
    last_name = Unicode()
    
    dog = Instance(Dog)
    
    def _default_dog(self):
        return Dog(last_name=self.last_name, owner=self)
    
    
bob = Person(first_name='Bob', last_name='Smith')
fido = bob.dog
fido.first_name = 'Fido'

print fido.owner.first_name, fido.owner.last_name
print fido.first_name, fido.last_name
