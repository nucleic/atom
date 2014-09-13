#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
""" Functions to assist with formatting error messages.

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


def validation_message(prefix, member, atom, value):
    """ Create an error message for a ValidationError.

    Parameters
    ----------
    prefix : str
        The prefix string for the error message.

    member : Member
        The member involved in the error.

    atom : Atom
        The Atom object involved in the error.

    value : object
        The invalid member value.

    Returns
    -------
    result : str

    """
    type_name = add_article(type(atom).__name__)
    repr_value = '%r %r' % (value, type(value))
    msg = "%s of %s instance must be %s, but a value of %s was specified."
    return msg % (prefix, type_name, member.type_info, repr_value)


def member_message(member, atom, name, value):
    """ Create an error message for a member validation error.

    Parameters
    ----------
    member : Member
        The member involved in the error.

    atom : Atom
        The Atom object involved in the error.

    name : str
        The name of the member involved in the error.

    value : object
        The invalid member value.

    Returns
    -------
    result : str

    """
    prefix = "The '%s' member" % name
    return validation_message(prefix, member, atom, value)


def container_message(container, kind, name, value):
    """ Create an error message for a container validation error.

    Parameters
    ----------
    container : object
        The container object.

    kine : type
        The required type of the value.

    name : str
        The name to use when referring to the value.

    value : object
        The invalid container value.

    Returns
    -------
    result : str

    """
    msg = "Each %s of %s must be %s, but a value of %s was specified."
    container_name = add_article(type(container).__name__)
    kind_name = add_article(kind.__name__)
    value_name = '%r %r' % (value, type(value))
    return msg % (name, container_name, kind_name, value_name)
