# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
""" Demonstrate the use of Compostion of Atom objects.

1. If the class has not been declared, use a ForwardTyped
   - Note the use of lambda, because "Person" is not yet defined

2. A Typed object can be instantiated three ways:
   - Provide args, kwargs, or a factory in the definition
   - Provide a _default_* static constructor
   - Provide a pre-created object in the constructor

"""
from atom.api import Atom, ForwardTyped, Str, Typed


class Dog(Atom):
    name = Str()

    # note the use of lambda, because Person has not been defined
    owner = ForwardTyped(lambda: Person)


class Person(Atom):
    name = Str()

    # uses static constructor
    # When using a static constructor the member is considered by default to be
    # optional even though it is often not the desired behavior, and specifying
    # optional=False preventing the member to be set to None makes sense.
    fido = Typed(Dog, optional=False)

    # uses kwargs provided in the definition
    # When the member is provided a way to build a default value, it assumes it
    # is not optional by default, i.e. None is not a valid value.
    fluffy = Typed(Dog, kwargs={"name": "Fluffy"})

    # uses an object provided in Person constructor
    new_dog = Typed(Dog)

    def _default_fido(self):
        return Dog(name="Fido", owner=self)


if __name__ == "__main__":
    bob = Person(name="Bob Smith")

    print("Fido")
    print("name: {0}".format(bob.fido.name))
    assert bob.fido.owner  # owner is optional so check it is set
    print("owner: {0}".format(bob.fido.owner.name))

    print("\nFluffy")
    print("name: {0}".format(bob.fluffy.name))
    print("original owner: {0}".format(repr(bob.fluffy.owner)))  # none
    bob.fluffy.owner = bob
    print("new owner: {0}".format(bob.fluffy.owner.name))

    print("\nNew Dog")
    new_dog = Dog(name="Scruffy", owner=bob)
    bob.new_dog = new_dog
    print("name: {0}".format(bob.new_dog.name))
    assert bob.new_dog.owner  # owner is optional so check it is set
    print("owner: {0}".format(bob.new_dog.owner.name))
