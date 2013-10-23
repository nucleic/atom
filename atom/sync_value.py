#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------


class SyncObserver(object):
    """ A callable object used to implement synchronized observers.

    """
    __slots__ = ('obj', 'attr')

    def __init__(self, obj, attr):
        """ Initialize an SyncObserver.

        Parameters
        ----------
        obj : Atom
            The object to syncrhonize the value with.

        attr : str
            The attribute name on the other object which should be
            observed.

        """
        self.obj = obj
        self.attr = attr

    def __call__(self, change):
        """ Handle a change of the original object.

        This handler will set the attribute on the synchronized object.

        """
        if change['type'] == 'update':
            setattr(self.obj, self.attr, change['value'])


def find_sync_observer(member, obj, attr):
    """Find a sync observer for a member watching a given obj and attr

    """
    for obs in member.static_observers():
        if isinstance(obs, SyncObserver):
            if obs.obj == obj and obs.attr == attr:
                return obs


def sync_value(source, attr, other, alias=None, mutual=True, remove=False):
    """ Synchronize an attribute between to Atoms

    Parameters
    ----------
    source : Atom
        The source object.
    attr : str
        The name of the parameter to synchronize.
    other : Atom
        The other object to sync the value with.
    alias : str, optional
        The name of the parameter in the other object, if different.
    mutual : bool, optional (True)
        Whether to synchronize in both directions.
    remove: bool, optional (False)
        If True, remove the synchroniziation.
    """
    if alias is None:
       alias = attr
    mbr1 = source.get_member(attr)
    if not mbr1:
        return
    mbr2 = other.get_member(alias)
    if not mbr2:
        return
    obs1 = find_sync_observer(mbr1, other, alias)
    obs2 = find_sync_observer(mbr2, source, attr)
    if remove:
        if obs1:
            mbr1.remove_static_observer(obs1)
        if obs2:
            mbr2.remove_static_observer(obs2)
        return
    # if there is already an observer, use it
    if not obs1:
        obs1 = SyncObserver(other, alias)
        mbr1.add_static_observer(obs1)
    # sync the value right away, if set
    val1 = getattr(source, attr, None)
    if val1:
        setattr(other, alias, val1)
    if mutual:
        if not obs2:
            obs2 = SyncObserver(source, attr)
            mbr2.add_static_observer(obs2)
        val2 = getattr(other, alias, None)
        if val2:
            setattr(source, attr, val2)
