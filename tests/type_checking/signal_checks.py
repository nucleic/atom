from typing import assert_type

from atom.api import Atom, Signal
from atom.catom import SignalConnector


# Signal members are descriptor objects on the class and a connector object on the
# instance.
class A(Atom):
    m = Signal()


assert_type(A.m, Signal)
assert_type(A().m, SignalConnector)
A().m()
