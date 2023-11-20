Welcome to Atom
===============

.. image:: https://github.com/nucleic/atom/workflows/Continuous%20Integration/badge.svg
    :target: https://github.com/nucleic/atom/actions
.. image:: https://github.com/nucleic/atom/workflows/Documentation%20building/badge.svg
    :target: https://github.com/nucleic/atom/actions
.. image:: https://codecov.io/gh/nucleic/atom/branch/main/graph/badge.svg
    :target: https://codecov.io/gh/nucleic/atom
.. image:: https://readthedocs.org/projects/atom/badge/?version=latest
    :target: https://atom.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status
.. image:: https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/astral-sh/ruff/main/assets/badge/v2.json
    :target: https://github.com/astral-sh/ruff
    :alt: Ruff

Atom is a framework for creating memory efficient Python objects with enhanced
features such as dynamic initialization, validation, and change notification for
object attributes. It provides the default model binding behavior for the
`Enaml <https://enaml.readthedocs.io/en/latest/>`_ UI framework.

Examples:

.. code-block:: python

    from atom.api import Atom, Str, Range, Bool, observe


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

Starting with atom 0.8.0 atom object can also be defined using type annotations.

.. code-block:: python

    from atom.api import Atom, observe

    class InventoryItem(Atom):
        """Class for keeping track of an item in inventory."""

        name: str
        unit_price: float
        quantity_on_hand: int = 0

        def total_cost(self) -> float:
            return self.unit_price * self.quantity_on_hand

        @observe("unit_price")
        def check_for_price_reduction(self, change):
            savings = change.get("oldvalue", 0) - change.get("value")
            if savings > 0:
                print(f"Save ${savings} now on {self.name}s!")

    >>> w = InventoryItem(name="widget", unit_price=1.99, quantity_on_hand=10)
    >>> w.unit_price = 1.00
    Save $0.99 now on widgets!


For version information, see `the Revision History <https://github.com/nucleic/atom/blob/main/releasenotes.rst>`_.
