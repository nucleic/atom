#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from atom.api import Atom, Unicode, Range


class Company(Atom):
    
    name = Unicode()
    
    address = Unicode()
    
    city = Unicode()
  
      
class Employee(Atom):
    """ A simple class representing an employee object.

    """
    first_name = Unicode()
    
    last_name = Unicode()

    age = Range(low=0)
    
    work_address = Unicode()
    
    city = Unicode()
    
        
bob = Employee(first_name='Robert', last_name='Paulson', age=40)
acme = Company(name='Acme', address='101 West Palm', city='Boca Raton')

bob.sync_value('work_address', acme, 'address')
bob.sync_value('city', acme)

print bob.work_address
print bob.city
