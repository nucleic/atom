#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from atom.api import Atom, Unicode, Range, DelegatesTo


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
    
    company = Company(name='Acme')
    
    work_address = DelegatesTo(company, 'address')
    
    city = DelegatesTo(company)
    
        
bob = Employee(first_name='Robert', last_name='Paulson', age=40)
bob.company.address = '101 West Palm'
print bob.work_address
print bob.company.address
bob.city = 'Boca Raton'
print bob.city
print bob.company.city
