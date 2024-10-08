# --------------------------------------------------------------------------------------
# Copyright (c) 2020-2024, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the examples."""

import os
import runpy

import pytest

example_folder = os.path.join(os.path.dirname(__file__), "..", "examples")
examples = []
for dirpath, dirnames, filenames in os.walk(example_folder):
    examples += [os.path.join(dirpath, f) for f in filenames]


@pytest.mark.parametrize("path", examples)
def test_example(path):
    if "pickling" in path:
        pytest.skip("Example requires to be executed as a top level module")
    runpy.run_path(path)
