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

from atom.api import Atom, Dict, List, Set

try:
    import psutil

    PSUTIL_UNAVAILABLE = False
except ImportError:
    PSUTIL_UNAVAILABLE = True

TIMEOUT = 10


class DictObj(Atom):
    data = Dict(default={"a": 0})


class ListObj(Atom):
    data = List(default=[1, 2, 3])


class SetObj(Atom):
    data = Set(default={1, 2, 3})


MEM_TESTS = {
    "dict": DictObj,
    "list": ListObj,
    "set": SetObj,
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


@pytest.mark.skipif(PSUTIL_UNAVAILABLE, reason="psutil is not installed")
@pytest.mark.parametrize("label", MEM_TESTS.keys())
def test_mem_usage(label):
    TestClass = MEM_TESTS[label]
    p = Process(target=memtest, args=(TestClass,))
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
