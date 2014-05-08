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

from atom.api import Atom, Dict, Float, Unicode, atomdict

TEST_DICT = {'a': 1, 'b': 2, 'c': 3}
KEY_DICT = {u'a': 1, u'b': 2, u'c': 3}
VAL_DICT = {'a': 1.0, 'b': 2.0, 'c': 3.0}
TYPED_DICT = {u'a': 1.0, u'b': 2.0, u'c': 3.0}

class StandardModel(Atom):
    """ A model class for testing dict behavior.

    """
    #: A standard dict with no type checking.
    untyped = Dict()

    #: A standard dict with Unicode keys.
    key_typed = Dict(Unicode())

    #: A standard dict with Float values.
    value_typed = Dict(value=Float())
    
    #: A standard dict with Unicode keys and Float values.
    typed = Dict(Unicode(), Float())


class DictTestBase(object):
    """ A base class which provides untyped base dict tests.

    """
    #: Set this to one of the models defined above in setUp.
    model = None

    #--------------------------------------------------------------------------
    # Untyped Tests
    #--------------------------------------------------------------------------
    def test_untyped_any_data(self):
        data = {object(): (), object: [], type: {}, 12.0: 10, 'wfoo': 9j,
                9j: 'wfoo', 10: 12.0, (): type}
        self.model.untyped = data
        eq_(self.model.untyped, data)

    def test_untyped_convert_to_dict(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        eq_(dict(self.model.untyped), data)

    def test_untyped_iterate(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        keys = [i for i in self.model.untyped]
        eq_(keys, data.keys())

    def test_untyped_copy_on_assign(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        eq_(self.model.untyped, data)
        ok_(self.model.untyped is not data)

    def test_untyped_items(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        eq_(self.model.untyped.items(), data.items())
        
    def test_untyped_keys(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        eq_(self.model.untyped.keys(), data.keys())
        
    def test_untyped_values(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        eq_(self.model.untyped.values(), data.values())
        
    def test_untyped_setdefault(self):
        eq_(None, self.model.untyped.setdefault('a'))
        eq_(self.model.untyped, {'a': None})
        eq_(1, self.model.untyped.setdefault('b',  1))
        eq_(self.model.untyped, {'a': None, 'b': 1})
        eq_(1, self.model.untyped.setdefault('b',  20))
        eq_(self.model.untyped, {'a': None, 'b': 1})
        
    def test_untyped_update(self):
        eq_(None, self.model.untyped.update({'a': 1}))
        eq_(self.model.untyped, {'a': 1})
        self.model.untyped.update((('b', 1), ('c', 2)))
        eq_(self.model.untyped, {'a': 1, 'b': 1, 'c': 2})
        self.model.untyped.update([('b', 2), ('c', 2)])
        eq_(self.model.untyped, {'a': 1, 'b': 2, 'c': 2})
        self.model.untyped.update([['b', 3], ['c', 3]])
        eq_(self.model.untyped, {'a': 1, 'b': 3, 'c': 3})
        self.model.untyped.update(a=10)
        eq_(self.model.untyped, {'a': 10, 'b': 3, 'c': 3})
        
    def test_untyped_get(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        eq_(1, self.model.untyped.get(1))
        eq_(None, self.model.untyped.get(10))
        
    def test_untyped_clear(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        self.model.untyped.clear()
        eq_({}, self.model.untyped)
        
    def test_untyped_copy(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        aux = self.model.untyped.copy()
        eq_(aux, self.model.untyped)
        
    def test_untyped_has_key(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        assert self.model.untyped.has_key(0)

    def test_untyped_pop(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        self.model.untyped.pop(9)
        eq_(self.model.untyped, {i: i**2 for i in range(9)})
        self.model.untyped.pop(0)
        eq_(self.model.untyped, {i: i**2 for i in range(1, 9)})
        
    def test_untyped_popitem(self):
        data = {i: i**2 for i in range(10)}
        self.model.untyped = data
        key, val = self.model.untyped.popitem()
        del data[key]
        eq_(self.model.untyped, data)

    def test_untyped_get_item(self):
        self.model.untyped = {i: i**2 for i in range(5)}
        eq_(self.model.untyped[3], 9)

    def test_untyped_set_item(self):
        self.model.untyped = {i: i**2 for i in range(5)}
        self.model.untyped[5] = 42
        eq_(self.model.untyped[5], 42)

    def test_untyped_del_item(self):
        self.model.untyped = {i: i**2 for i in range(5)}
        del self.model.untyped[3]
        eq_(self.model.untyped, {0: 0, 1: 1, 2: 4, 4: 16})
        
    #--------------------------------------------------------------------------
    # Key typed Tests
    #--------------------------------------------------------------------------
    def test_key_typed_convert_to_dict(self):
        self.model.key_typed = TEST_DICT.copy()
        eq_(dict(self.model.key_typed), TEST_DICT)

    def test_key_typed_iterate(self):
        self.model.key_typed = TEST_DICT.copy()
        data = [i for i in self.model.key_typed]
        eq_(data, TEST_DICT.keys())

    def test_key_typed_copy_on_assign(self):
        data = TEST_DICT.copy()
        self.model.key_typed = data
        eq_(self.model.key_typed, data)
        ok_(self.model.key_typed is not data)

    def test_key_typed_items(self):
        data = TEST_DICT.copy()
        self.model.key_typed = data
        eq_(self.model.key_typed.items(), data.items())
        
    def test_key_typed_keys(self):
        data = TEST_DICT.copy()
        self.model.key_typed = data
        eq_(self.model.key_typed.keys(), data.keys())
        
    def test_key_typed_values(self):
        data = TEST_DICT.copy()
        self.model.key_typed = data
        eq_(self.model.key_typed.values(), data.values())
        
    def test_key_typed_setdefault(self):
        eq_(1, self.model.key_typed.setdefault('b',  1))
        eq_(self.model.key_typed, {u'b': 1})
        eq_(1, self.model.key_typed.setdefault('b',  20))
        eq_(self.model.key_typed, {u'b': 1})
        
    def test_key_typed_update(self):
        eq_(None, self.model.key_typed.update({'a': 1}))
        eq_(self.model.key_typed, {u'a': 1})
        self.model.key_typed.update((('b', 1), ('c', 2)))
        eq_(self.model.key_typed, {u'a': 1, u'b': 1, u'c': 2})
        self.model.key_typed.update([('b', 2), ('c', 2)])
        eq_(self.model.key_typed, {u'a': 1, u'b': 2, u'c': 2})
        self.model.key_typed.update([['b', 3], ['c', 3]])
        eq_(self.model.key_typed, {u'a': 1, u'b': 3, u'c': 3})
        self.model.key_typed.update(a=10)
        eq_(self.model.key_typed, {u'a': 10, 'b': 3, 'c': 3})
        
    def test_key_typed_get(self):
        self.model.key_typed = TEST_DICT.copy()
        eq_(1, self.model.key_typed.get('a'))
        eq_(None, self.model.key_typed.get(10))
        
    def test_key_typed_clear(self):
        self.model.key_typed = TEST_DICT.copy()
        self.model.key_typed.clear()
        eq_({}, self.model.key_typed)
        
    def test_key_typed_copy(self):
        self.model.key_typed = TEST_DICT.copy()
        aux = self.model.key_typed.copy()
        eq_(aux, self.model.key_typed)
        
    def test_key_typed_has_key(self):
        self.model.key_typed = TEST_DICT.copy()
        assert self.model.key_typed.has_key(u'a')

    def test_key_typed_pop(self):
        data = TEST_DICT.copy()
        self.model.key_typed = data
        self.model.key_typed.pop('a')
        data.pop('a')
        eq_(self.model.key_typed, data)
        
    def test_key_typed_popitem(self):
        data = TEST_DICT.copy()
        self.model.key_typed = data
        key, val = self.model.key_typed.popitem()
        del data[key]
        eq_(self.model.key_typed, data)

    def test_key_typed_get_item(self):
        self.model.key_typed = TEST_DICT.copy()
        eq_(self.model.key_typed[u'a'], 1)

    def test_key_typed_set_item(self):
        self.model.key_typed = TEST_DICT.copy()
        self.model.key_typed['c'] = 42
        eq_(self.model.key_typed['c'], 42)
        self.model.key_typed['d'] = 42
        eq_(self.model.key_typed['d'], 42)

    def test_key_typed_del_item(self):
        self.model.key_typed = TEST_DICT.copy()
        del self.model.key_typed[u'a']
        eq_(self.model.key_typed, {u'b': 2, u'c': 3})
        
    #--------------------------------------------------------------------------
    # Value typed Tests
    #--------------------------------------------------------------------------
    def test_value_typed_convert_to_dict(self):
        self.model.value_typed = TEST_DICT.copy()
        eq_(dict(self.model.value_typed), TEST_DICT)

    def test_value_typed_iterate(self):
        self.model.value_typed = TEST_DICT.copy()
        data = [i for i in self.model.value_typed]
        eq_(data, TEST_DICT.keys())

    def test_value_typed_copy_on_assign(self):
        data = TEST_DICT.copy()
        self.model.value_typed = data
        eq_(self.model.value_typed, data)
        ok_(self.model.value_typed is not data)

    def test_value_typed_items(self):
        data = TEST_DICT.copy()
        self.model.value_typed = data
        eq_(self.model.value_typed.items(), data.items())
        
    def test_value_typed_keys(self):
        data = TEST_DICT.copy()
        self.model.value_typed = data
        eq_(self.model.value_typed.keys(), data.keys())
        
    def test_value_typed_values(self):
        data = TEST_DICT.copy()
        self.model.value_typed = data
        eq_(self.model.value_typed.values(), data.values())
        
    def test_value_typed_setdefault(self):
        eq_(1, self.model.value_typed.setdefault('b',  1))
        eq_(self.model.value_typed, {'b': 1.0})
        eq_(1, self.model.value_typed.setdefault('b',  20))
        eq_(self.model.value_typed, {'b': 1.0})
        
    def test_value_typed_update(self):
        eq_(None, self.model.value_typed.update({'a': 1}))
        eq_(self.model.value_typed, {'a': 1.0})
        self.model.value_typed.update((('b', 1), ('c', 2)))
        eq_(self.model.value_typed, {'a': 1.0, 'b': 1.0, 'c': 2.0})
        self.model.value_typed.update([('b', 2), ('c', 2)])
        eq_(self.model.value_typed, {'a': 1.0, 'b': 2.0, 'c': 2.0})
        self.model.value_typed.update([['b', 3], ['c', 3]])
        eq_(self.model.value_typed, {'a': 1.0, 'b': 3.0, 'c': 3.0})
        self.model.value_typed.update(a=10)
        eq_(self.model.value_typed, {'a': 10.0, 'b': 3.0, 'c': 3.0})
        
    def test_value_typed_get(self):
        self.model.value_typed = TEST_DICT.copy()
        eq_(1.0, self.model.value_typed.get('a'))
        eq_(None, self.model.value_typed.get(10))
        
    def test_value_typed_clear(self):
        self.model.value_typed = TEST_DICT.copy()
        self.model.value_typed.clear()
        eq_({}, self.model.value_typed)
        
    def test_value_typed_copy(self):
        self.model.value_typed = TEST_DICT.copy()
        aux = self.model.value_typed.copy()
        eq_(aux, self.model.value_typed)
        
    def test_value_typed_has_key(self):
        self.model.value_typed = TEST_DICT.copy()
        assert self.model.value_typed.has_key('a')

    def test_value_typed_pop(self):
        data = TEST_DICT.copy()
        self.model.value_typed = data
        self.model.value_typed.pop('a')
        data.pop('a')
        eq_(self.model.value_typed, data)
        
    def test_value_typed_popitem(self):
        data = TEST_DICT.copy()
        self.model.value_typed = data
        key, val = self.model.value_typed.popitem()
        del data[key]
        eq_(self.model.value_typed, data)

    def test_value_typed_get_item(self):
        self.model.value_typed = TEST_DICT.copy()
        eq_(self.model.value_typed[u'a'], 1)

    def test_value_typed_set_item(self):
        self.model.value_typed = TEST_DICT.copy()
        self.model.value_typed['c'] = 42
        eq_(self.model.value_typed['c'], 42.0)
        self.model.value_typed['d'] = 42
        eq_(self.model.value_typed['d'], 42.0)

    def test_value_typed_del_item(self):
        self.model.value_typed = TEST_DICT.copy()
        del self.model.value_typed['a']
        eq_(self.model.value_typed, {'b': 2.0, 'c': 3.0})

    #--------------------------------------------------------------------------
    # Fully typed Tests
    #--------------------------------------------------------------------------
    def test_typed_convert_to_dict(self):
        self.model.typed = TEST_DICT.copy()
        eq_(dict(self.model.typed), TEST_DICT)

    def test_typed_iterate(self):
        self.model.typed = TEST_DICT.copy()
        data = [i for i in self.model.typed]
        eq_(data, TEST_DICT.keys())

    def test_typed_copy_on_assign(self):
        data = TEST_DICT.copy()
        self.model.typed = data
        eq_(self.model.typed, data)
        ok_(self.model.typed is not data)

    def test_typed_items(self):
        data = TEST_DICT.copy()
        self.model.typed = data
        eq_(self.model.typed.items(), data.items())
        
    def test_typed_keys(self):
        data = TEST_DICT.copy()
        self.model.typed = data
        eq_(self.model.typed.keys(), data.keys())
        
    def test_typed_values(self):
        data = TEST_DICT.copy()
        self.model.typed = data
        eq_(self.model.typed.values(), data.values())
        
    def test_typed_setdefault(self):
        eq_(1, self.model.typed.setdefault('b',  1))
        eq_(self.model.typed, {u'b': 1.0})
        eq_(1, self.model.typed.setdefault('b',  20))
        eq_(self.model.typed, {u'b': 1.0})
        
    def test_typed_update(self):
        eq_(None, self.model.typed.update({'a': 1}))
        eq_(self.model.typed, {u'a': 1.0})
        self.model.typed.update((('b', 1), ('c', 2)))
        eq_(self.model.typed, {u'a': 1.0, u'b': 1.0, u'c': 2.0})
        self.model.typed.update([('b', 2), ('c', 2)])
        eq_(self.model.typed, {u'a': 1.0, u'b': 2.0, u'c': 2.0})
        self.model.typed.update([['b', 3], ['c', 3]])
        eq_(self.model.typed, {u'a': 1.0, u'b': 3.0, u'c': 3.0})
        self.model.typed.update(a=10)
        eq_(self.model.typed, {u'a': 10.0, u'b': 3.0, u'c': 3.0})
        
    def test_typed_get(self):
        self.model.typed = TEST_DICT.copy()
        eq_(1.0, self.model.typed.get(u'a'))
        eq_(None, self.model.typed.get(10))
        
    def test_typed_clear(self):
        self.model.typed = TEST_DICT.copy()
        self.model.typed.clear()
        eq_({}, self.model.typed)
        
    def test_typed_copy(self):
        self.model.typed = TEST_DICT.copy()
        aux = self.model.typed.copy()
        eq_(aux, self.model.typed)
        
    def test_typed_has_key(self):
        self.model.typed = TEST_DICT.copy()
        assert self.model.typed.has_key(u'a')

    def test_typed_pop(self):
        data = TEST_DICT.copy()
        self.model.typed = data
        self.model.typed.pop('a')
        data.pop('a')
        eq_(self.model.typed, data)
        
    def test_typed_popitem(self):
        data = TEST_DICT.copy()
        self.model.typed = data
        key, val = self.model.typed.popitem()
        del data[key]
        eq_(self.model.typed, data)

    def test_typed_get_item(self):
        self.model.typed = TEST_DICT.copy()
        eq_(self.model.typed[u'a'], 1)

    def test_typed_set_item(self):
        self.model.typed = TEST_DICT.copy()
        self.model.typed['c'] = 42
        eq_(self.model.typed['c'], 42.0)
        self.model.typed['d'] = 42
        eq_(self.model.typed['d'], 42.0)

    def test_typed_del_item(self):
        self.model.typed = TEST_DICT.copy()
        del self.model.typed['a']
        eq_(self.model.typed, {'b': 2.0, 'c': 3.0})


class TestStandardDict(DictTestBase):
    """ A test class for the List member.

    """
    def setUp(self):
        self.model = StandardModel()

    def tearDown(self):
        self.model = None

    def test_dict_types(self):
        eq_(type(self.model.untyped), atomdict)
        eq_(type(self.model.key_typed), atomdict)
        eq_(type(self.model.value_typed), atomdict)
        eq_(type(self.model.typed), atomdict)

    def test_pickle(self):
        data = TEST_DICT.copy()
        self.model.untyped = data
        self.model.key_typed = data
        self.model.value_typed = data
        self.model.typed = data
        eq_(data, loads(dumps(self.model.untyped, 0)))
        eq_(data, loads(dumps(self.model.untyped, 1)))
        eq_(data, loads(dumps(self.model.untyped, 2)))
        eq_(KEY_DICT, loads(dumps(self.model.key_typed, 0)))
        eq_(KEY_DICT, loads(dumps(self.model.key_typed, 1)))
        eq_(KEY_DICT, loads(dumps(self.model.key_typed, 2)))
        eq_(VAL_DICT, loads(dumps(self.model.value_typed, 0)))
        eq_(VAL_DICT, loads(dumps(self.model.value_typed, 1)))
        eq_(VAL_DICT, loads(dumps(self.model.value_typed, 2)))
        eq_(TYPED_DICT, loads(dumps(self.model.typed, 0)))
        eq_(TYPED_DICT, loads(dumps(self.model.typed, 1)))
        eq_(TYPED_DICT, loads(dumps(self.model.typed, 2)))

    @raises(TypeError)
    def test_key_typed_bad_setdefault(self):
        self.model.key_typed.setdefault(1)

    @raises(TypeError)
    def test_key_typed_bad_update(self):
        self.model.key_typed.update({1: 1})

    @raises(TypeError)
    def test_key_typed_bad_set_item(self):
        self.model.typed = TEST_DICT.copy()
        self.model.typed[5] = 42j
        
    @raises(TypeError)
    def test_value_typed_bad_setdefault(self):
        self.model.value_typed.setdefault(1, 'a')

    @raises(TypeError)
    def test_value_typed_bad_update(self):
        self.model.value_typed.update({1: 'a'})

    @raises(TypeError)
    def test_value_typed_bad_set_item(self):
        self.model.typed = TEST_DICT.copy()
        self.model.typed['a'] = 'e'

    @raises(TypeError)
    def test_typed_bad_key_setdefault(self):
        self.model.typed.setdefault(1.0)
        
    @raises(TypeError)
    def test_typed_bad_value_setdefault(self):
        self.model.typed.setdefault('a', 'e')

    @raises(TypeError)
    def test_typed_bad_key_update(self):
        self.model.typed.update({1: 1})
        
    @raises(TypeError)
    def test_typed_bad_value_update(self):
        self.model.typed.update({'a': 'e'})

    @raises(TypeError)
    def test_typed_bad_key_set_item(self):
        self.model.typed = TEST_DICT.copy()
        self.model.typed[5] = 42
        
    @raises(TypeError)
    def test_typed_bad_value_set_item(self):
        self.model.typed = TEST_DICT.copy()
        self.model.typed['a'] = 'e'


#class TestContainerList(TestStandardList):
#    """ A test class for the ContainerList.
#
#    """
#    change = None
#
#    def setUp(self):
#        self.model = ContainerModel()
#
#    def tearDown(self):
#        self.model = None
#
#    def test_list_types(self):
#        eq_(type(self.model.untyped), atomclist)
#        eq_(type(self.model.typed), atomclist)
#
#def containertest(func):
#    @wraps(func)
#    def closure(self, name):
#        mlist = getattr(self.model, name)
#        func(self, mlist)
#        self.verify_base_change(name, mlist)
#    return closure
#
#
#class TestContainerNotify(object):
#    """ A test class for the ContainerList.
#
#    """
#    model = None
#
#    change = None
#
#    def setUp(self):
#        self.model = ContainerModel()
#        self.model.untyped = range(10)
#        self.model.typed = range(10)
#        self.model.observe('untyped', self._changed)
#        self.model.observe('typed', self._changed)
#
#    def tearDown(self):
#        self.model.unobserve('untyped', self._changed)
#        self.model.unobserve('typed', self._changed)
#        self.model = None
#
#    def _changed(self, change):
#        self.change = change
#
#    def verify_base_change(self, name, mlist):
#        eq_(self.change['type'], 'container')
#        eq_(self.change['name'], name)
#        eq_(self.change['object'], self.model)
#        eq_(self.change['value'], mlist)
#
#    @containertest
#    def container_append(self, mlist):
#        mlist.append(1)
#        eq_(self.change['operation'], 'append')
#        eq_(self.change['item'], 1)
#
#    def test_container_append(self):
#        yield (self.container_append, 'untyped')
#        yield (self.container_append, 'typed')
#
#    @containertest
#    def container_insert(self, mlist):
#        mlist.insert(0, 42)
#        eq_(self.change['operation'], 'insert')
#        eq_(self.change['index'], 0)
#        eq_(self.change['item'], 42)
#
#    def test_container_insert(self):
#        yield (self.container_insert, 'untyped')
#        yield (self.container_insert, 'typed')
#
#    @containertest
#    def container_extend(self, mlist):
#        mlist.extend(range(3))
#        eq_(self.change['operation'], 'extend')
#        eq_(self.change['items'], range(3))
#
#    def test_container_extend(self):
#        yield (self.container_extend, 'untyped')
#        yield (self.container_extend, 'typed')
#
#    @containertest
#    def container_remove(self, mlist):
#        mlist.remove(5)
#        eq_(self.change['operation'], 'remove')
#        eq_(self.change['item'], 5)
#
#    def test_container_remove(self):
#        yield (self.container_remove, 'untyped')
#        yield (self.container_remove, 'typed')
#
#    @containertest
#    def container_pop(self, mlist):
#        mlist.pop(0)
#        eq_(self.change['operation'], 'pop')
#        eq_(self.change['index'], 0)
#        eq_(self.change['item'], 0)
#
#    def test_container_pop(self):
#        yield (self.container_pop, 'untyped')
#        yield (self.container_pop, 'typed')
#
#    @containertest
#    def container_reverse(self, mlist):
#        mlist.reverse()
#        eq_(self.change['operation'], 'reverse')
#
#    def test_container_reverse(self):
#        yield (self.container_reverse, 'untyped')
#        yield (self.container_reverse, 'typed')
#
#    @containertest
#    def container_sort(self, mlist):
#        mlist.sort()
#        eq_(self.change['operation'], 'sort')
#        eq_(self.change['cmp'], None)
#        eq_(self.change['key'], None)
#        eq_(self.change['reverse'], False)
#
#    @containertest
#    def container_key_sort(self, mlist):
#        key = lambda i: i
#        mlist.sort(key=key, reverse=True)
#        eq_(self.change['operation'], 'sort')
#        eq_(self.change['cmp'], None)
#        eq_(self.change['key'], key)
#        eq_(self.change['reverse'], True)
#
#    @containertest
#    def container_cmp_sort(self, mlist):
#        cmpfunc = lambda a, b: a < b
#        mlist.sort(cmp=cmpfunc, reverse=True)
#        eq_(self.change['operation'], 'sort')
#        eq_(self.change['cmp'], cmpfunc)
#        eq_(self.change['key'], None)
#        eq_(self.change['reverse'], True)
#
#    def test_container_sort(self):
#        yield (self.container_sort, 'untyped')
#        yield (self.container_sort, 'typed')
#        yield (self.container_key_sort, 'untyped')
#        yield (self.container_key_sort, 'typed')
#        yield (self.container_cmp_sort, 'untyped')
#        yield (self.container_cmp_sort, 'typed')
#
#    @containertest
#    def container_set_item(self, mlist):
#        mlist[0] = 42
#        eq_(self.change['operation'], '__setitem__')
#        eq_(self.change['index'], 0)
#        eq_(self.change['olditem'], 0)
#        eq_(self.change['newitem'], 42)
#
#    def test_container_set_item(self):
#        yield (self.container_set_item, 'untyped')
#        yield (self.container_set_item, 'typed')
#
#    @containertest
#    def container_set_slice(self, mlist):
#        mlist[3:5] = [1, 2, 3]
#        eq_(self.change['operation'], '__setitem__')
#        eq_(self.change['index'], slice(3, 5, None))
#        eq_(self.change['olditem'], [3, 4])
#        eq_(self.change['newitem'], [1, 2, 3])
#
#    def test_container_set_slice(self):
#        yield (self.container_set_slice, 'untyped')
#        yield (self.container_set_slice, 'typed')
#
#    @containertest
#    def container_set_slice_step(self, mlist):
#        mlist[::2] = [1, 2, 3, 4, 5]
#        eq_(self.change['operation'], '__setitem__')
#        eq_(self.change['index'], slice(None, None, 2))
#        eq_(self.change['olditem'], [0, 2, 4, 6, 8])
#        eq_(self.change['newitem'], [1, 2, 3, 4, 5])
#
#    def test_container_set_slice_step(self):
#        yield (self.container_set_slice_step, 'untyped')
#        yield (self.container_set_slice_step, 'typed')
#
#    @containertest
#    def container_del_item(self, mlist):
#        del mlist[0]
#        eq_(self.change['operation'], '__delitem__')
#        eq_(self.change['index'], 0)
#        eq_(self.change['item'], 0)
#
#    def test_container_del_item(self):
#        yield (self.container_del_item, 'untyped')
#        yield (self.container_del_item, 'typed')
#
#    @containertest
#    def container_del_slice(self, mlist):
#        del mlist[0:5]
#        eq_(self.change['operation'], '__delitem__')
#        eq_(self.change['index'], slice(0, 5, None))
#        eq_(self.change['item'], range(5))
#
#    def test_container_del_slice(self):
#        yield (self.container_del_slice, 'untyped')
#        yield (self.container_del_slice, 'typed')
#
#    @containertest
#    def container_del_slice_step(self, mlist):
#        del mlist[::2]
#        eq_(self.change['operation'], '__delitem__')
#        eq_(self.change['index'], slice(None, None, 2))
#        eq_(self.change['item'], range(10)[::2])
#
#    def test_container_del_slice_step(self):
#        yield (self.container_del_slice_step, 'untyped')
#        yield (self.container_del_slice_step, 'typed')
#
#    @containertest
#    def container_concat(self, mlist):
#        mlist += [1, 2, 3]
#        eq_(self.change['operation'], '__iadd__')
#        eq_(self.change['items'], [1, 2, 3])
#
#    def test_container_concat(self):
#        yield (self.container_concat, 'untyped')
#        yield (self.container_concat, 'typed')
#
#    @containertest
#    def container_repeat(self, mlist):
#        mlist *= 2
#        eq_(self.change['operation'], '__imul__')
#        eq_(self.change['count'], 2)
#
#    def test_container_repeat(self):
#        yield (self.container_repeat, 'untyped')
#        yield (self.container_repeat, 'typed')
