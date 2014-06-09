from atom.atom import *


class Foo(Atom):
    foo = Member()
    bar = Member()
    apple = Member()
    mellon = Member()
    green = Member()
    yellow = Member()
    blue = Member()


class Bar(object):
    pass


foo = Foo()
bar = Bar()

foo.foo = 1
foo.bar = 2
foo.apple = 3
foo.mellon = 4
foo.green = 5
foo.yellow = 6
foo.blue = 7

bar.foo = 1
bar.bar = 2
bar.apple = 3
bar.mellon = 4
bar.green = 5
bar.yellow = 6
bar.blue = 7
