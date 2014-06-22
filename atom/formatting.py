#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
""" A collection of functions to assist in formatting error messages.

"""
def add_article(name):
    """ Prefix the given name with the proper article 'a' or 'an'.

    """
    if name[0].lower() in 'aeiou':
        return 'an ' + name
    return 'a ' + name


def kind_repr(kind, prefix):
    """ Create a string description for a kind argument.

    """
    result = prefix
    if not isinstance(kind, tuple):
        return result + kind.__name__
    result += kind[0].__name__
    if len(kind) == 1:
        return result
    if len(kind) == 2:
        return result + " or " + kind[1].__name__
    for k in kind[1:-1]:
        result += ", " + k.__name__
    result += ", or " + kind[-1].__name__
    return result


def instance_repr(kind):
    """ Create a string description for an isinstance() kind argument.

    """
    return kind_repr(kind, 'an instance of ')


def subclass_repr(kind):
    """ Create a string description for an issubclass() kind argument.

    """
    return kind_repr(kind, 'a subclass of ')


def coerced_repr(kind):
    """ Create a string description for a coerceable kind argument.

    """
    return kind_repr(kind, 'coercible to ')


def validation_message(prefix, atom, value, type_info):
    """ Create an error message for a ValidationError.

    Parameters
    ----------
    prefix : str
        The prefix string for the error message.

    atom : Atom
        The Atom object involved in the error.

    value : object
        The invalid member value.

    type_info : str
        The type info associated with the member.

    """
    type_name = add_article(type(atom).__name__)
    repr_value = '%r %r' % (value, type(value))
    msg = "%s of %s instance must be %s, but a value of %s was specified."
    return msg % (prefix, type_name, type_info, repr_value)


def member_message(atom, name, value, type_info):
    """ Create an error message for a member validation error.

    Parameters
    ----------
    atom : Atom
        The Atom object involved in the error.

    name : str
        The name of the member involved in the error.

    value : object
        The invalid member value.

    type_info : str
        The type info associated with the member.

    """
    prefix = "The '%s' member" % name
    return validation_message(prefix, atom, value, type_info)


def element_message(atom, name, value, type_info):
    """ Create an error message for an element validation error.

    Parameters
    ----------
    atom : Atom
        The Atom object involved in the error.

    name : str
        The name of the member involved in the error.

    value : object
        The invalid member value.

    type_info : str
        The type info associated with the member.

    """
    prefix = "Each element of the '%s' member" % name
    return validation_message(prefix, atom, value, type_info)
