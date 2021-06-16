# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import Member, GetAttr, SetAttr, DelAttr

class Signal(Member):  # XXX should we disallow get, set, delete
    def __call__(cls, *args, **kwargs) -> None: ...
