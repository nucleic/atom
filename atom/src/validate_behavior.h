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


namespace Validate
{

enum Mode
{
    NoOp,
    Bool,
    Int,
    IntCast,
    Float,
    FloatCast,
    Bytes,
    String,
    Tuple,
    List,
    ContainerList,
    Dict,
    Instance,
    Typed,
    Subclass,
    Enum,
    Callable,
    Range,
    FloatRange,
    Coerced,
    ObjectMethod,
    MemberMethod,
    Str,            // deprecated
    StrPromote,     // deprecated
    Unicode,        // deprecated
    UnicodePromote, // deprecated
    Last            // sentinel
};

} // namespace Validate


// true on success, false and exception on failure
bool Validate_CheckMode( Validate::Mode mode, PyObject* context );


// new ref on success, null and exception on failure
PyObject* Member_Validate( Member* member,
                           CAtom* atom,
                           PyStringObject* name,
                           PyObject* value );
