from typing import assert_type

from atom.api import Atom, List


# Annotated member declarations should expose the Atom descriptor type on the class
# and the corresponding concrete Python container on the instance.
class A(Atom):
    m: List[int] = List()


assert_type(A.m, List[int])
assert_type(A().m, list[int])
