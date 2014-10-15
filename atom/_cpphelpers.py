#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
""" A module of helper functions for the C++ catom module.

This module must not import anything which imports catom.

"""
from .formatting import kind_repr


def _value_message_common(container, value_type, value):
    """ Create a validation error message for value.

    This is a common message function for tuple, list, and set.

    """
    type_repr = kind_repr(value_type)
    value_repr = '%r %r' % (value, type(value))
    fmt = ("Each element of the %s must be an instance of %s, "
           "but a value of %s was specified.")
    return fmt % (container, type_repr, value_repr)


def typed_tuple_validation_message(value_type, value):
    """ Create a validation error message for a tuple object.

    Parameters
    ----------
    value_type : type or tuple of types
        The allowed type or types for the tuple elements.

    value : object
        The tuple element value which failed validation.

    Returns
    -------
    result : str
        The message to use with a ValidationError.

    """
    return _value_message_common('tuple', value_type, value)


def typed_list_validation_message(typed_list, value):
    """ Create a validation error message for a TypedList object.

    Parameters
    ----------
    typed_list : TypedList
        The typed list object of interest.

    value : object
        The list element value which failed validation.

    Returns
    -------
    result : str
        The message to use with a ValidationError.

    """
    return _value_message_common('list', typed_list.value_type, value)


def typed_set_validation_message(typed_set, value):
    """ Create a validation error message for a TypedSet object.

    Parameters
    ----------
    typed_set : TypedSet
        The typed set object of interest.

    value : object
        The set element value which failed validation.

    Returns
    -------
    result : str
        The message to use with a ValidationError.

    """
    return _value_message_common('set', typed_set.value_type, value)


def typed_dict_validation_message(typed_dict, item):
    """ Create a validation error message for a TypedList object.

    Parameters
    ----------
    typed_list : TypedList
        The typed list object of interest.

    item : tuple
        The (key, value) item which failed validation.

    Returns
    -------
    result : str
        The message to use with a ValidationError.

    """
    key, value = item
    key_type_repr = kind_repr(typed_dict.key_type)
    value_type_repr = kind_repr(typed_dict.value_type)
    item_repr = '(%r %r, %r %r)' % (key, type(key), value, type(value))
    fmt = ("Each item of the dict must have key type %s, "
           "and value type %s, but the item %s was specified.")
    return fmt % (key_type_repr, value_type_repr, item_repr)
