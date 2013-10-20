#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from atom.api import Atom, Unicode
import sys

  
def get_last_name():
    '''Return a last name based on the system byteorder'''
    return sys.byteorder.capitalize()

      
class Person(Atom):
    """ A simple class representing an employee object.

    """
    first_name = Unicode()
    
    last_name = Unicode()

    def _default_last_name(self):
        return get_last_name()
    
        
bob = Person(first_name='Robert')
print bob.first_name, bob.last_name
