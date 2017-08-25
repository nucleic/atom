#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

import datetime

from atom.api import (
    Atom, Unicode, Range, Bool, Value, Int, Tuple, Typed, observe
)


class Person(Atom):
    """ A simple class representing a person object.

    """
    last_name = Unicode()

    first_name = Unicode()

    age = Range(low=0)

    dob = Value(datetime.date(1970, 1, 1))

    debug = Bool(False)

    @observe('age')
    def debug_print(self, change):
        """ Prints out a debug message whenever the person's age changes.

        """
        if self.debug:
            templ = "{first} {last} is {age} years old."
            s = templ.format(
                first=self.first_name, last=self.last_name, age=self.age,
            )
            print(s)


class Employer(Person):
    """ An employer is a person who runs a company.

    """
    # The name of the company
    company_name = Unicode()


class Employee(Person):
    """ An employee is person with a boss and a phone number.

    """
    # The employee's boss
    boss = Typed(Employer)

    # The employee's phone number as a tuple of 3 ints
    phone = Tuple(Int())

    # This method will be called automatically by atom when the
    # employee's phone number changes
    def _observe_phone(self, val):
        if val['type'] == 'update':
            msg = 'received new phone number for %s: %s'
            print(msg % (self.first_name, val['value']))


if __name__ == '__main__':
    # Create an employee with a boss
    boss_john = Employer(
        first_name='John', last_name='Paw', company_name="Packrat's Cats",
    )
    employee_mary = Employee(
        first_name='Mary', last_name='Sue', boss=boss_john,
        phone=(555, 555, 5555),
    )

    employee_mary.phone = (100, 100, 100)
    employee_mary.age = 40  # no debug message
    employee_mary.debug = True
    employee_mary.age = 50
