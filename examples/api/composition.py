#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from atom.api import (
    Atom, Value, Instance, ForwardInstance, Unicode)


class Dog(Atom):

    name = Unicode()

    owner = ForwardInstance(lambda: Person)


class Person(Atom):

    name = Unicode()

    dog = Instance(Dog)

    def _default_dog(self):
        return Dog(name='Fido', owner=self)


class Dogsitter(Atom):

    name = Unicode()

    dog = Value(Dog)


if __name__ == '__main__':

    bob = Person(name='Bob Smith')
    print bob.dog
    print bob.dog.owner == bob

    peg = Dogsitter(name='Peggy')
    peg.dog = bob.dog
    print peg.dog
    print peg.dog == bob.dog
