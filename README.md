Welcome to Atom
===============

Atom is a framework for creating memory efficient Python objects with enhanced 
features such as dynamic initialization, validation, and change notification for 
object attributes. It provides the default model binding behaviour for the
[Enaml](http://nucleic.github.io/enaml/docs) UI framework.

Illustrative Example::
   
    from atom.api import Atom, Unicode, Range, Bool, observe


    class Person(Atom):
        """ A simple class representing a person object.

        """
        last_name = Unicode()

        first_name = Unicode()

        age = Range(low=0)

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
                
        def _default_first_name(self):
            return 'John'


    john = Person(last_name='Doe', age=42)
    john.debug = True
    john.age = 43  # prints message
    john.age = 'forty three'   # raises TypeError
                
