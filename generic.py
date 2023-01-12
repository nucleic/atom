from typing import Generic, TypeVar
from atom.api import Atom, Float

T = TypeVar("T")

class A(Atom, Generic[T]):
    t : T

class B(A[float]):
  pass

assert isinstance(B.t, Float)