from atom.api import Atom
from atom.api import observe


class InventoryItem(Atom):
    """Class for keeping track of an item in inventory."""

    name: str
    unit_price: float
    quantity_on_hand: int = 0

    def total_cost(self) -> float:
        return self.unit_price * self.quantity_on_hand

    @observe("unit_price")
    def _observe_unit_price(self, change):
        savings = change.get("oldvalue", 0) - change.get("value")
        if savings > 0:
            print(f"Save ${savings} now on {self.name}s!")


if __name__ == "__main__":
    w = InventoryItem(name="widget", unit_price=1.99, quantity_on_hand=10)

    w.unit_price = 1.00
