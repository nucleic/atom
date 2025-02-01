/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2024, Nucleic Development Team.
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
    Last // sentinel
};

}  // namespace GetState

constexpr uint8_t _required_handler_size(uint8_t x) {
    return (
        (x > 128) ? 255 :
        (x > 64) ? 128 :
        (x > 32) ? 64 :
        (x > 16) ? 32 :
        (x > 8) ? 16 :
        (x > 4) ? 8 :
        (x > 2) ? 4 :
        2
    );
}

#define _handlers_size( a ) ( sizeof(a) / sizeof(a[0]) )
#define validate_handlers( handlers, sentinel ) \
    _handlers_size(handlers) - 1; \
    static_assert(sentinel <= _handlers_size(handlers), "Not enough handlers for all enum values"); \
    static_assert(_required_handler_size(sentinel) == _handlers_size(handlers), "Handlers size does not match enum width") \

}  // namespace atom
