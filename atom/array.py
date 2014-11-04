#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .coerced import Coerced


class Array(Coerced):
    """ A value of type `np.ndarray`

    Values are coerced to ndarrays using np.array.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None, kwargs=None):
        """ Initialize the Array, coercing to an ndarray if necessary.
        
        """
        import numpy as np
        if factory is None:
            if default is not None:
                kwargs = kwargs or {}
                factory = lambda: np.array(default, **kwargs)
            else:
                factory = lambda: ()
        super(Array, self).__init__(
                np.ndarray, factory=factory, coercer=np.array)
