#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from atom.api import Atom, Unicode, Range, observe


class Dog(Atom):

    first_name = Unicode()


class Person(Atom):
    """ A simple class representing a person object.

    """
    first_name = Unicode()

    last_name = Unicode()

    age = Range(low=0)

    dog = Dog(first_name='Fido')

    def _observe_first_name(self, change):
        '''Demonstrate static method observer convention
        '''
        print 'First name changed:', change['value']

    @observe('age')
    def debug_print(self, change):
        '''Demonstrate simple observe decorator
        '''
        print 'Age changed:', change['value']

    @observe('dog.first_name')
    def print_dog_first_name(self, change):
        '''Demonstrate observation of another Atom's attribute
        '''
        print 'Dog first name changed:', change['value']


bob = Person(first_name='Robert', last_name='Paulson', age=40)
bob.first_name = 'Bobby'
bob.age = 50
bob.dog.first_name = 'Scruffy'

