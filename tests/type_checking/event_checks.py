from typing_extensions import assert_type

from atom.api import Atom, Event
from atom.catom import EventBinder


# Event members keep the Event descriptor on the class but bind an EventBinder on the
# instance; assignment is permitted because the binder is the runtime object.
class A(Atom):
    m = Event(int)


assert_type(A.m, Event[int])
assert_type(A().m, EventBinder)
A().m = 1
