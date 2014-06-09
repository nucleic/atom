/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>


struct CAtom;
struct Member;


namespace DefaultValue
{

enum Mode
{
    NoOp,
    Static,
    List,
    Dict,
    CallObject,
    ObjectMethod,
    MemberMethod,
    Last // sentinel
};

} // namespace DefaultValue


// true on success, false and exception on failure
bool DefaultValue_CheckMode( DefaultValue::Mode mode, PyObject* context );


// new ref on success, null and exception on failure
PyObject*
Member_DefaultValue( Member* member, CAtom* atom, PyStringObject* name );
