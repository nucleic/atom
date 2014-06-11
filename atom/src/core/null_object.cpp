/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "null_object.h"


namespace atom
{

namespace
{

struct NullObjectHelper
{
    static PyObject Object;
    static PyTypeObject TypeObject;
};


void tp_dealloc( PyObject* ignore )
{
    Py_FatalError( "deallocating null" );
}


PyObject* tp_repr( PyObject* op )
{
    return PyString_FromString( "null" );
}


PyTypeObject NullObjectHelper::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type ) /* header */
    0,                                 /* ob_size */
    "NullType",                        /* tp_name */
    0,                                 /* tp_basicsize */
    0,                                 /* tp_itemsize */
    tp_dealloc,                        /* tp_dealloc ( never called ) */
    0,                                 /* tp_print */
    0,                                 /* tp_getattr */
    0,                                 /* tp_setattr */
    0,                                 /* tp_compare */
    tp_repr,                           /* tp_repr */
    0,                                 /* tp_as_number */
    0,                                 /* tp_as_sequence */
    0,                                 /* tp_as_mapping */
    ( hashfunc )_Py_HashPointer,       /* tp_hash */
};


PyObject NullObjectHelper::Object = {
    _PyObject_EXTRA_INIT
    1,
    &NullObjectHelper::TypeObject
};

} // namespace


PyObject* NullObject = &NullObjectHelper::Object;

} // namespace atom
