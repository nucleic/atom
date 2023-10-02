# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import time
from multiprocessing import Process

import pytest

from atom.api import Atom, DefaultDict, Dict, Int, List, Set, atomref

try:
    import psutil

    PSUTIL_UNAVAILABLE = False
except ImportError:
    PSUTIL_UNAVAILABLE = True

TIMEOUT = 10


class DictObj(Atom):
    data = Dict(default={"a": 0})


class DefaultDictObj(Atom):
    data = DefaultDict(value=Int(), default={1: 1})  # type: ignore


class ListObj(Atom):
    data = List(default=[1, 2, 3])


class SetObj(Atom):
    data = Set(default={1, 2, 3})


class RefObj(Atom):
    data = Int()


MEM_TESTS = {
    "dict": DictObj,
    "defaultdict": DefaultDictObj,
    "list": ListObj,
    "set": SetObj,
    "atomref": RefObj,
}


def memtest(cls):
    # Create object in a loop
    # Memory usage should settle out and not cange
    t = time.time()

    while True:
        obj = cls()
        obj.data  # Force creation
        del obj
        if time.time() - t > TIMEOUT:
            break


def atomreftest(cls):
    t = time.time()
    obj = cls()
    obj.data
    while True:
        ref = atomref(obj)  # noqa
        del ref
        if time.time() - t > TIMEOUT:
            break


@pytest.mark.skipif(PSUTIL_UNAVAILABLE, reason="psutil is not installed")
@pytest.mark.parametrize("label", MEM_TESTS.keys())
def test_mem_usage(label):
    TestClass = MEM_TESTS[label]
    if "atomref" in label:
        target = atomreftest
    else:
        target = memtest

    p = Process(target=target, args=(TestClass,))
    p.start()
    try:
        stats = psutil.Process(p.pid)
        time.sleep(TIMEOUT * 1 / 4)
        first_info = stats.memory_info()
        time.sleep(TIMEOUT * 3 / 4)
        last_info = stats.memory_info()
        assert first_info == last_info, "Memory leaked:\n  {}\n  {}".format(
            first_info, last_info
        )
    finally:
        p.kill()
