/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2025, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

namespace atom
{


namespace GetAttr
{

enum Mode: uint8_t
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

enum Mode: uint8_t
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

enum Mode: uint8_t
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

enum Mode: uint8_t
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

enum Mode: uint8_t
{
    NoOp,
    Static,
    List,
    Set,
    Dict,
    DefaultDict,
    NonOptional,
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

enum Mode: uint8_t
{
    NoOp,
    Bool,
    Int,
    IntPromote,
    Float,
    FloatPromote,
    Bytes,
    BytesPromote,
    Str,
    StrPromote,
    Tuple,
    FixedTuple,
    List,
    ContainerList,
    Set,
    Dict,
    DefaultDict,
    OptionalInstance,
    Instance,
    OptionalTyped,
    Typed,
    Subclass,
    Enum,
    Callable,
    FloatRange,
    FloatRangePromote,
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

enum Mode: uint8_t
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

enum Mode: uint8_t
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


namespace GetState
{

enum Mode: uint8_t
{
    Include,  // We want include to be the default behavior
    Exclude,
    IncludeNonDefault,
    Property,
    ObjectMethod_Name,
    MemberMethod_Object,
};

}  // namespace GetState

}  // namespace atom
