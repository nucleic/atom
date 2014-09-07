from atom.api import *
import threading
import time


l = threading.Lock()


class Foo1(Atom):
    a = Signal()


class Foo2(Atom):
    a = Signal()


class Bar1(Atom):
    a = Signal()


class Bar2(Atom):
    a = Signal()


f1 = Foo1()
f2 = Foo2()
b1 = Bar1()
b2 = Bar2()


def p1():
    s = ('foo2', Atom.sender())
    with l:
        print s



def p2():
    for i in range(10):
        s = ('foo1', Atom.sender())
        with l:
            print s
        time.sleep(0.001)
        f2.a()
        s = ('foo1', Atom.sender())
        with l:
            print s


def p3():
    s = ('bar2', Atom.sender())
    with l:
        print s


def p4():
    for i in range(10):
        s = ('bar1', Atom.sender())
        with l:
            print s
        time.sleep(0.001)
        b2.a()
        s = ('bar1', Atom.sender())
        with l:
            print s


f1.a.connect(p2)
f2.a.connect(p1)


b1.a.connect(p4)
b2.a.connect(p3)


def worker(ob):
    ob.a()


t = threading.Thread(target=worker, args=(f1,))
t.daemon = True
t.start()
worker(b1)
