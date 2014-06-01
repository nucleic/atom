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
    IntPromote,
    Long,
    LongPromote,
    Float,
    FloatPromote,
    Str,
    StrPromote,
    Unicode,
    UnicodePromote,
    Tuple,
    List,
    ContainerList,
    Dict,
    Instance,
    Typed,
    Subclass,
    Enum,
    Callable,
    FloatRange,
    Range,
    Coerced,
    CallObject,
    ObjectMethod,
    MemberMethod,
    Last // sentinel
};

} // namespace Validate


// true on success, false and exception on failure
bool Validate_CheckMode( Validate::Mode mode, PyObject* context );


// new ref on success, null and exception on failure
PyObject* Member_Validate( Member* member,
                           CAtom* atom,
                           PyStringObject* name,
                           PyObject* oldvalue,
                           PyObject* newvalue );
