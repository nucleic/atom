/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once


namespace GetAttr
{

enum Mode
{
    NoOp,
    Slot,
    Event,
    Signal,
    Delegate,
    Property,
    CachedProperty,
    CallObject_Object,
    CallObject_ObjectName,
    ObjectMethod,
    ObjectMethod_Name,
    MemberMethod_Object,
    Last // sentinel
};

} // namespace GetAttr


namespace PostGetAttr
{

enum Mode
{
    NoOp,
    Delegate,
    ObjectMethod_Value,
    ObjectMethod_NameValue,
    MemberMethod_ObjectValue,
    Last // sentinel
};

} // namespace PostGetAttr


namespace SetAttr
{

enum Mode
{
    NoOp,
    Slot,
    Constant,
    ReadOnly,
    Event,
    Signal,
    Delegate,
    Property,
    CallObject_ObjectValue,
    CallObject_ObjectNameValue,
    ObjectMethod_Value,
    ObjectMethod_NameValue,
    MemberMethod_ObjectValue,
    Last // sentinel
};

} // namespace SetAttr


namespace PostSetAttr
{

enum Mode
{
    NoOp,
    Delegate,
    ObjectMethod_OldNew,
    ObjectMethod_NameOldNew,
    MemberMethod_ObjectOldNew,
    Last // sentinel
};

} // namespace PostSetAttr


namespace DefaultValue
{

enum Mode
{
    NoOp,
    Static,
    List,
    Dict,
    Delegate,
    CallObject,
    CallObject_Object,
    CallObject_ObjectName,
    ObjectMethod,
    ObjectMethod_Name,
    MemberMethod_Object,
    Last // sentinel
};

} // namespace DefaultValue


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
    Bytes,
    BytesPromote,
    String,
    StringPromote,
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
    Delegate,
    ObjectMethod_OldNew,
    ObjectMethod_NameOldNew,
    MemberMethod_ObjectOldNew,
    Last // sentinel
};

} // namespace Validate


namespace PostValidate
{

enum Mode
{
    NoOp,
    Delegate,
    ObjectMethod_OldNew,
    ObjectMethod_NameOldNew,
    MemberMethod_ObjectOldNew,
    Last // sentinel
};

} // namespace PostValidate


namespace DelAttr
{

enum Mode
{
    NoOp,
    Slot,
    Constant,
    ReadOnly,
    Event,
    Signal,
    Delegate,
    Property,
    Last // sentinel
};

} // namespace DelAttr
