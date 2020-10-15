Welcome to Atom
===============

.. image:: https://github.com/nucleic/atom/workflows/Continuous%20Integration/badge.svg
    :target: https://github.com/nucleic/atom/actions
.. image:: https://github.com/nucleic/atom/workflows/Documentation%20building/badge.svg
    :target: https://github.com/nucleic/atom/actions
.. image:: https://codecov.io/gh/nucleic/atom/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/nucleic/atom
.. image:: https://readthedocs.org/projects/atom/badge/?version=latest
    :target: https://atom.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

Atom is a framework for creating memory efficient Python objects with enhanced
features such as dynamic initialization, validation, and change notification for
object attributes. It provides the default model binding behaviour for the
`Enaml <http://nucleic.github.io/enaml/docs>`_ UI framework.

Version 0.4.3 will be the last version to support Python 2.  Moving forward
support will be limited to Python 3.5+.

Illustrative Example:

.. code-block:: python

    from atom.api import Atom, Unicode, Range, Bool, observe


    class Person(Atom):
        """ A simple class representing a person object.

        """
        last_name = Str()

        first_name = Str()

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

For version information, see `the Revision History <https://github.com/nucleic/atom/blob/master/releasenotes.rst>`_.