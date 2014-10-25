#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
""" A collection of message formatting utility functions.

"""


def add_article(noun):
    """ Prefix a noun with the proper article 'a' or 'an'.

    Parameters
    ----------
    noun : str
        The noun to be prefixed with a proper article.

    Returns
    -------
    result : str
        The prefixed noun.

    """
    if noun[0].lower() in 'aeiou':
        return 'an ' + noun
    return 'a ' + noun


def collect_names(names, kind):
    """ Fill a list with names from a type or tuple of types.

    Parameters
    ----------
    names : list
        The list which holds the type names. This is modified in-place.

    kind : type or tuple of types
        The type(s) of interest. The semantics of this argument is
        the same as the second argument to the builtin isinstance().

    """
    if isinstance(kind, type):
        names.append(kind.__name__)
        return
    for k in kind:
        collect_names(names, k)


def kind_repr(kind):
    """ Create a string description for a kind argument.

    Parameters
    ----------
    kind : type or tuple of types
        The type(s) of interest. The semantics of this argument is
        the same as the second argument to the builtin isinstance().

    Returns
    -------
    result : str
        The expanding string representation.

    """
    names = []
    collect_names(names, kind)
    count = len(names)
    if count == 0:
        return ''
    if count == 1:
        return names[0]
    if count == 2:
        return names[0] + " or " + names[1]
    result = names[0]
    for name in names[1:-1]:
        result += ", " + name
    result += ", or " + names[-1]
    return result
