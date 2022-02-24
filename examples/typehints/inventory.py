# --------------------------------------------------------------------------------------
# Copyright (c) 2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Example on using type hints to create Atom subclasses."""
from atom.api import Atom, ChangeDict, observe


class InventoryItem(Atom):
    """Class for keeping track of an item in inventory.

    This example demonstrates creating an Atom using python type hints
    and using `observe` to detect when an item goes on sale.

    Expected output: `Save $0.99 now on widgets!`
    """

    name: str
    unit_price: float
    quantity_on_hand: int = 0

    def total_cost(self) -> float:
        return self.unit_price * self.quantity_on_hand

    @observe("unit_price")
    def _observe_unit_price(self, change: ChangeDict) -> None:
        savings = change.get("oldvalue", 0) - change["value"]
        if savings > 0:
            print(f"Save ${savings} now on {self.name}s!")


if __name__ == "__main__":
    w = InventoryItem(name="widget", unit_price=1.99, quantity_on_hand=10)

    w.unit_price = 1.00
