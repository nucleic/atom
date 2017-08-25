#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
""" Demonstrate the use of Compostion of Atom objects.

1. If the class has not been declared, use a ForwardTyped
   - Note the use of lambda, because "Person" is not yet defined

2. A Typed object can be instantiated three ways:
   - Provide args, kwargs, or a factory in the definition
   - Provide a _default_* static constructor
   - Provide a pre-created object in the constructor

"""
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

from atom.api import Atom, Typed, ForwardTyped, Unicode


class Dog(Atom):

    name = Unicode()

    # note the use of lambda, because Person has not been defined
    owner = ForwardTyped(lambda: Person)


class Person(Atom):

    name = Unicode()

    # uses static constructor
    fido = Typed(Dog)

    # uses kwargs provided in the definition
    fluffy = Typed(Dog, kwargs=dict(name='Fluffy'))

    # uses an object provided in Person constructor
    new_dog = Typed(Dog)

    def _default_fido(self):
        return Dog(name='Fido', owner=self)


if __name__ == '__main__':
    bob = Person(name='Bob Smith')

    print('Fido')
    print('name: {0}'.format(bob.fido.name))
    print('owner: {0}'.format(bob.fido.owner.name))

    print('\nFluffy')
    print('name: {0}'.format(bob.fluffy.name))
    print('original owner: {0}'.format(repr(bob.fluffy.owner)))  # none
    bob.fluffy.owner = bob
    print('new owner: {0}'.format(bob.fluffy.owner.name))

    print('\nNew Dog')
    new_dog = Dog(name='Scruffy', owner=bob)
    bob.new_dog = new_dog
    print('name: {0}'.format(bob.new_dog.name))
    print('owner: {0}'.format(bob.new_dog.owner.name))
