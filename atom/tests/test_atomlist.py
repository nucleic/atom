#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from cPickle import dumps, loads
from functools import wraps

from nose.tools import eq_, ok_, raises

from atom.api import Atom, List, Int, ContainerList, atomlist, atomclist


class StandardModel(Atom):
    """ A model class for testing atomlist behavior.

    """
    #: A standard list with no type checking.
    untyped = List()

    #: A standard list of integers.
    typed = List(Int())


class ContainerModel(Atom):
    """ A model class for testing atomclist behavior.

    """
    #: A container list with no type checking.
    untyped = ContainerList()

    #: A container list of integers.
    typed = ContainerList(Int())


class ListTestBase(object):
    """ A base class which provides untyped base list tests.

    """
    #: Set this to one of the models defined above in setUp.
    model = None

    #--------------------------------------------------------------------------
    # Untyped Tests
    #--------------------------------------------------------------------------
    def test_untyped_any_data(self):
        data = [object(), object, type, 12.0, 'wfoo', 9j, 10, {}, [], ()]
        self.model.untyped = data
        eq_(self.model.untyped, data)

    def test_untyped_convert_to_list(self):
        self.model.untyped = range(10)
        eq_(list(self.model.untyped), range(10))

    def test_untyped_iterate(self):
        self.model.untyped = range(10)
        data = [i for i in self.model.untyped]
        eq_(data, range(10))

    def test_untyped_copy_on_assign(self):
        data = range(10)
        self.model.untyped = data
        eq_(self.model.untyped, data)
        ok_(self.model.untyped is not data)

    def test_untyped_append(self):
        self.model.untyped.append(1)
        eq_(self.model.untyped, [1])

    def test_untyped_extend(self):
        self.model.untyped.extend(range(10))
        eq_(self.model.untyped, range(10))

    def test_untyped_insert(self):
        self.model.untyped = range(10)
        self.model.untyped.insert(0, 19)
        eq_(self.model.untyped, [19] + range(10))

    def test_untyped_remove(self):
        self.model.untyped = range(10)
        self.model.untyped.remove(5)
        data = range(10)
        data.remove(5)
        eq_(self.model.untyped, data)

    def test_untyped_pop(self):
        self.model.untyped = range(10)
        self.model.untyped.pop()
        eq_(self.model.untyped, range(9))
        self.model.untyped.pop(0)
        eq_(self.model.untyped, range(1, 9))

    def test_untyped_index(self):
        self.model.untyped = range(10)
        index = self.model.untyped.index(5)
        eq_(index, 5)

    def test_untyped_count(self):
        self.model.untyped = [1] * 10
        count = self.model.untyped.count(1)
        eq_(count, 10)

    def test_untyped_reverse(self):
        self.model.untyped = range(10)
        self.model.untyped.reverse()
        eq_(self.model.untyped, list(reversed(range(10))))

    def test_untyped_sort(self):
        self.model.untyped = [8, 3, 2, 5, 9]
        self.model.untyped.sort()
        eq_(self.model.untyped, [2, 3, 5, 8, 9])
        self.model.untyped.sort(reverse=True)
        eq_(self.model.untyped, [9, 8, 5, 3, 2])

    def test_untyped_get_item(self):
        self.model.untyped = range(10)
        eq_(self.model.untyped[3], 3)

    def test_untyped_get_slice(self):
        self.model.untyped = range(10)
        eq_(self.model.untyped[3:8], range(3, 8))

    def test_untyped_get_slice_step(self):
        self.model.untyped = range(10)
        eq_(self.model.untyped[3::2], range(3, 10, 2))

    def test_untyped_set_item(self):
        self.model.untyped = range(10)
        self.model.untyped[5] = 42
        eq_(self.model.untyped[5], 42)

    def test_untyped_set_slice(self):
        self.model.untyped = range(5)
        self.model.untyped[3:5] = [42, 42]
        eq_(self.model.untyped, [0, 1, 2, 42, 42])

    def test_untyped_set_slice_step(self):
        self.model.untyped = range(5)
        self.model.untyped[::2] = [42, 42, 42]
        eq_(self.model.untyped, [42, 1, 42, 3, 42])

    def test_untyped_del_item(self):
        self.model.untyped = range(5)
        del self.model.untyped[3]
        eq_(self.model.untyped, [0, 1, 2, 4])

    def test_untyped_del_slice(self):
        self.model.untyped = range(5)
        del self.model.untyped[3:]
        eq_(self.model.untyped, range(3))

    def test_untyped_del_slice_step(self):
        self.model.untyped = range(10)
        del self.model.untyped[::2]
        eq_(self.model.untyped, range(1, 10, 2))

    def test_untyped_concat(self):
        self.model.untyped = range(10)
        self.model.untyped += range(5)
        eq_(self.model.untyped, range(10) + range(5))

    def test_untyped_repeat(self):
        self.model.untyped = range(10)
        self.model.untyped *= 3
        eq_(self.model.untyped, range(10) * 3)

    #--------------------------------------------------------------------------
    # Typed Tests
    #--------------------------------------------------------------------------
    def test_typed_convert_to_list(self):
        self.model.typed = range(10)
        eq_(list(self.model.typed), range(10))

    def test_typed_iterate(self):
        self.model.typed = range(10)
        data = [i for i in self.model.typed]
        eq_(data, range(10))

    def test_typed_copy_on_assign(self):
        data = range(10)
        self.model.typed = data
        eq_(self.model.typed, data)
        ok_(self.model.typed is not data)

    def test_typed_append(self):
        self.model.typed.append(1)
        eq_(self.model.typed, [1])

    def test_typed_extend(self):
        self.model.typed.extend(range(10))
        eq_(self.model.typed, range(10))

    def test_typed_insert(self):
        self.model.typed = range(10)
        self.model.typed.insert(0, 19)
        eq_(self.model.typed, [19] + range(10))

    def test_typed_remove(self):
        self.model.typed = range(10)
        self.model.typed.remove(5)
        data = range(10)
        data.remove(5)
        eq_(self.model.typed, data)

    def test_typed_pop(self):
        self.model.typed = range(10)
        self.model.typed.pop()
        eq_(self.model.typed, range(9))
        self.model.typed.pop(0)
        eq_(self.model.typed, range(1, 9))

    def test_typed_index(self):
        self.model.typed = range(10)
        index = self.model.typed.index(5)
        eq_(index, 5)

    def test_typed_count(self):
        self.model.typed = [1] * 10
        count = self.model.typed.count(1)
        eq_(count, 10)

    def test_typed_reverse(self):
        self.model.typed = range(10)
        self.model.typed.reverse()
        eq_(self.model.typed, list(reversed(range(10))))

    def test_typed_sort(self):
        self.model.typed = [8, 3, 2, 5, 9]
        self.model.typed.sort()
        eq_(self.model.typed, [2, 3, 5, 8, 9])
        self.model.typed.sort(reverse=True)
        eq_(self.model.typed, [9, 8, 5, 3, 2])

    def test_typed_get_item(self):
        self.model.typed = range(10)
        eq_(self.model.typed[3], 3)

    def test_typed_get_slice(self):
        self.model.typed = range(10)
        eq_(self.model.typed[3:8], range(3, 8))

    def test_typed_get_slice_step(self):
        self.model.typed = range(10)
        eq_(self.model.typed[3::2], range(3, 10, 2))

    def test_typed_set_item(self):
        self.model.typed = range(10)
        self.model.typed[5] = 42
        eq_(self.model.typed[5], 42)

    def test_typed_set_slice(self):
        self.model.typed = range(5)
        self.model.typed[3:5] = [42, 42]
        eq_(self.model.typed, [0, 1, 2, 42, 42])

    def test_typed_set_slice_step(self):
        self.model.typed = range(5)
        self.model.typed[::2] = [42, 42, 42]
        eq_(self.model.typed, [42, 1, 42, 3, 42])

    def test_typed_del_item(self):
        self.model.typed = range(5)
        del self.model.typed[3]
        eq_(self.model.typed, [0, 1, 2, 4])

    def test_typed_del_slice(self):
        self.model.typed = range(5)
        del self.model.typed[3:]
        eq_(self.model.typed, range(3))

    def test_typed_del_slice_step(self):
        self.model.typed = range(10)
        del self.model.typed[::2]
        eq_(self.model.typed, range(1, 10, 2))

    def test_typed_concat(self):
        self.model.typed = range(10)
        self.model.typed += range(5)
        eq_(self.model.typed, range(10) + range(5))

    def test_typed_repeat(self):
        self.model.typed = range(10)
        self.model.typed *= 3
        eq_(self.model.typed, range(10) * 3)


class TestStandardList(ListTestBase):
    """ A test class for the List member.

    """
    def setUp(self):
        self.model = StandardModel()

    def tearDown(self):
        self.model = None

    def test_list_types(self):
        eq_(type(self.model.untyped), atomlist)
        eq_(type(self.model.typed), atomlist)

    def test_pickle(self):
        data = range(10)
        self.model.untyped = data
        self.model.typed = data
        eq_(data, loads(dumps(self.model.untyped, 0)))
        eq_(data, loads(dumps(self.model.untyped, 1)))
        eq_(data, loads(dumps(self.model.untyped, 2)))
        eq_(data, loads(dumps(self.model.typed, 0)))
        eq_(data, loads(dumps(self.model.typed, 1)))
        eq_(data, loads(dumps(self.model.typed, 2)))

    @raises(TypeError)
    def test_typed_bad_append(self):
        self.model.typed.append(1.0)

    @raises(TypeError)
    def test_typed_bad_extend(self):
        self.model.typed.extend([1, 2, 3, 'four'])

    @raises(TypeError)
    def test_typed_bad_insert(self):
        self.model.typed = range(10)
        self.model.typed.insert(0, object())

    @raises(TypeError)
    def test_typed_bad_set_item(self):
        self.model.typed = range(10)
        self.model.typed[5] = 42j

    @raises(TypeError)
    def test_typed_bad_set_slice(self):
        self.model.typed = range(5)
        self.model.typed[3:5] = ['None', 'None']

    @raises(TypeError)
    def test_typed_bad_set_slice_step(self):
        self.model.typed = range(5)
        self.model.typed[::2] = [56.7, 56.7, 56.7]

    @raises(TypeError)
    def test_typed_bad_concat(self):
        self.model.typed = range(10)
        self.model.typed += [12, 14, 'bad']


class TestContainerList(TestStandardList):
    """ A test class for the ContainerList.

    """
    change = None

    def setUp(self):
        self.model = ContainerModel()

    def tearDown(self):
        self.model = None

    def test_list_types(self):
        eq_(type(self.model.untyped), atomclist)
        eq_(type(self.model.typed), atomclist)

def containertest(func):
    @wraps(func)
    def closure(self, name):
        mlist = getattr(self.model, name)
        func(self, mlist)
        self.verify_base_change(name, mlist)
    return closure


class TestContainerNotify(object):
    """ A test class for the ContainerList.

    """
    model = None

    change = None

    def setUp(self):
        self.model = ContainerModel()
        self.model.untyped = range(10)
        self.model.typed = range(10)
        self.model.observe('untyped', self._changed)
        self.model.observe('typed', self._changed)

    def tearDown(self):
        self.model.unobserve('untyped', self._changed)
        self.model.unobserve('typed', self._changed)
        self.model = None

    def _changed(self, change):
        self.change = change

    def verify_base_change(self, name, mlist):
        eq_(self.change['type'], 'container')
        eq_(self.change['name'], name)
        eq_(self.change['object'], self.model)
        eq_(self.change['value'], mlist)

    @containertest
    def container_append(self, mlist):
        mlist.append(1)
        eq_(self.change['operation'], 'append')
        eq_(self.change['item'], 1)

    def test_container_append(self):
        yield (self.container_append, 'untyped')
        yield (self.container_append, 'typed')

    @containertest
    def container_insert(self, mlist):
        mlist.insert(0, 42)
        eq_(self.change['operation'], 'insert')
        eq_(self.change['index'], 0)
        eq_(self.change['item'], 42)

    def test_container_insert(self):
        yield (self.container_insert, 'untyped')
        yield (self.container_insert, 'typed')

    @containertest
    def container_extend(self, mlist):
        mlist.extend(range(3))
        eq_(self.change['operation'], 'extend')
        eq_(self.change['items'], range(3))

    def test_container_extend(self):
        yield (self.container_extend, 'untyped')
        yield (self.container_extend, 'typed')

    @containertest
    def container_remove(self, mlist):
        mlist.remove(5)
        eq_(self.change['operation'], 'remove')
        eq_(self.change['item'], 5)

    def test_container_remove(self):
        yield (self.container_remove, 'untyped')
        yield (self.container_remove, 'typed')

    @containertest
    def container_pop(self, mlist):
        mlist.pop(0)
        eq_(self.change['operation'], 'pop')
        eq_(self.change['index'], 0)
        eq_(self.change['item'], 0)

    def test_container_pop(self):
        yield (self.container_pop, 'untyped')
        yield (self.container_pop, 'typed')

    @containertest
    def container_reverse(self, mlist):
        mlist.reverse()
        eq_(self.change['operation'], 'reverse')

    def test_container_reverse(self):
        yield (self.container_reverse, 'untyped')
        yield (self.container_reverse, 'typed')

    @containertest
    def container_sort(self, mlist):
        mlist.sort()
        eq_(self.change['operation'], 'sort')
        eq_(self.change['cmp'], None)
        eq_(self.change['key'], None)
        eq_(self.change['reverse'], False)

    @containertest
    def container_key_sort(self, mlist):
        key = lambda i: i
        mlist.sort(key=key, reverse=True)
        eq_(self.change['operation'], 'sort')
        eq_(self.change['cmp'], None)
        eq_(self.change['key'], key)
        eq_(self.change['reverse'], True)

    @containertest
    def container_cmp_sort(self, mlist):
        cmpfunc = lambda a, b: a < b
        mlist.sort(cmp=cmpfunc, reverse=True)
        eq_(self.change['operation'], 'sort')
        eq_(self.change['cmp'], cmpfunc)
        eq_(self.change['key'], None)
        eq_(self.change['reverse'], True)

    def test_container_sort(self):
        yield (self.container_sort, 'untyped')
        yield (self.container_sort, 'typed')
        yield (self.container_key_sort, 'untyped')
        yield (self.container_key_sort, 'typed')
        yield (self.container_cmp_sort, 'untyped')
        yield (self.container_cmp_sort, 'typed')

    @containertest
    def container_set_item(self, mlist):
        mlist[0] = 42
        eq_(self.change['operation'], '__setitem__')
        eq_(self.change['index'], 0)
        eq_(self.change['olditem'], 0)
        eq_(self.change['newitem'], 42)

    def test_container_set_item(self):
        yield (self.container_set_item, 'untyped')
        yield (self.container_set_item, 'typed')

    @containertest
    def container_set_slice(self, mlist):
        mlist[3:5] = [1, 2, 3]
        eq_(self.change['operation'], '__setitem__')
        eq_(self.change['index'], slice(3, 5, None))
        eq_(self.change['olditem'], [3, 4])
        eq_(self.change['newitem'], [1, 2, 3])

    def test_container_set_slice(self):
        yield (self.container_set_slice, 'untyped')
        yield (self.container_set_slice, 'typed')

    @containertest
    def container_set_slice_step(self, mlist):
        mlist[::2] = [1, 2, 3, 4, 5]
        eq_(self.change['operation'], '__setitem__')
        eq_(self.change['index'], slice(None, None, 2))
        eq_(self.change['olditem'], [0, 2, 4, 6, 8])
        eq_(self.change['newitem'], [1, 2, 3, 4, 5])

    def test_container_set_slice_step(self):
        yield (self.container_set_slice_step, 'untyped')
        yield (self.container_set_slice_step, 'typed')

    @containertest
    def container_del_item(self, mlist):
        del mlist[0]
        eq_(self.change['operation'], '__delitem__')
        eq_(self.change['index'], 0)
        eq_(self.change['item'], 0)

    def test_container_del_item(self):
        yield (self.container_del_item, 'untyped')
        yield (self.container_del_item, 'typed')

    @containertest
    def container_del_slice(self, mlist):
        del mlist[0:5]
        eq_(self.change['operation'], '__delitem__')
        eq_(self.change['index'], slice(0, 5, None))
        eq_(self.change['item'], range(5))

    def test_container_del_slice(self):
        yield (self.container_del_slice, 'untyped')
        yield (self.container_del_slice, 'typed')

    @containertest
    def container_del_slice_step(self, mlist):
        del mlist[::2]
        eq_(self.change['operation'], '__delitem__')
        eq_(self.change['index'], slice(None, None, 2))
        eq_(self.change['item'], range(10)[::2])

    def test_container_del_slice_step(self):
        yield (self.container_del_slice_step, 'untyped')
        yield (self.container_del_slice_step, 'typed')

    @containertest
    def container_concat(self, mlist):
        mlist += [1, 2, 3]
        eq_(self.change['operation'], '__iadd__')
        eq_(self.change['items'], [1, 2, 3])

    def test_container_concat(self):
        yield (self.container_concat, 'untyped')
        yield (self.container_concat, 'typed')

    @containertest
    def container_repeat(self, mlist):
        mlist *= 2
        eq_(self.change['operation'], '__imul__')
        eq_(self.change['count'], 2)

    def test_container_repeat(self):
        yield (self.container_repeat, 'untyped')
        yield (self.container_repeat, 'typed')
