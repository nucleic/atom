# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import gc
import os
import pickle
import sys
import time
from multiprocessing import Process

import pytest

from atom.api import Atom, DefaultDict, Dict, Int, List, Set, atomref

try:
    import psutil

    PSUTIL_UNAVAILABLE = False
except ImportError:
    PSUTIL_UNAVAILABLE = True

TIMEOUT = 6


class DictObj(Atom):
    data = Dict(default={"a": 0})


class DefaultDictObj(Atom):
    data = DefaultDict(value=Int(), default={1: 1})


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

PICKLE_MEM_TESTS = {
    "dict": DictObj,
    "defaultdict": DefaultDictObj,
    "list": ListObj,
    "set": SetObj,
}


def memtest(cls):
    # Create object in a loop
    # Memory usage should settle out and not change
    while True:
        obj = cls()
        obj.data  # Force creation
        del obj
        gc.collect()


def atomreftest(cls):
    obj = cls()
    obj.data
    while True:
        ref = atomref(obj)
        del ref
        gc.collect()


@pytest.mark.skipif(
    "CI" in os.environ and sys.platform.startswith("darwin"),
    reason="Flaky on MacOS CI runners",
)
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
        # Allow slight memory decrease over time to make tests more resilient
        if first_info != last_info:
            assert (
                first_info.rss >= last_info.rss >= 0
            ), "Memory leaked:\n  {}\n  {}".format(first_info, last_info)
            assert (
                first_info.vms >= last_info.vms >= 0
            ), "Memory leaked:\n  {}\n  {}".format(first_info, last_info)
    finally:
        p.kill()
        p.join()


# Those tests can be informative but are flaky
@pytest.mark.skipif(
    "CI" in os.environ and sys.platform.startswith("darwin"),
    reason="Flaky on MacOS CI runners",
)
@pytest.mark.skipif(PSUTIL_UNAVAILABLE, reason="psutil is not installed")
@pytest.mark.parametrize("label", PICKLE_MEM_TESTS.keys())
def test_pickle_mem_usage(label):
    TestClass = PICKLE_MEM_TESTS[label]

    obj = TestClass()
    obj.data
    proc = psutil.Process()

    for _ in range(100):
        pickle.loads(pickle.dumps(obj))

    gc.collect()
    ref = proc.memory_full_info().uss
    for _ in range(10000):
        pck = pickle.dumps(obj)
        del pck
    gc.collect()
    assert abs(proc.memory_full_info().uss - ref) < 10_000
    for i in range(10000):
        pck = pickle.dumps(obj)
        pickle.loads(pck)
        del pck
    gc.collect()
    assert abs(proc.memory_full_info().uss - ref) < 10_000
