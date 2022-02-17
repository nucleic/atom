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

Atom is a framework for creating memory efficient Python objects with enhanced
features such as dynamic initialization, validation, and change notification for
object attributes. It provides the default model binding behavior for the
`Enaml <https://enaml.readthedocs.io/en/latest/>`_ UI framework.

Version 0.4.3 will be the last version to support Python 2.  Moving forward
support will be limited to Python 3.5+.

Illustrative Example:

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
