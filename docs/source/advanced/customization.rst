.. _advanced-customization:

Member customization: advanced techniques
=========================================

.. include:: ../substitutions.sub

In the basics, we covered how to customize the behavior of a member by using
prefixed method in a class definition. However this is not the only way to
customize members. The following section will first describe how a member
determine the action to take at each step of the getting, setting process,
before giving more details about the behaviors taht can be used to build
custom members.

Members inner working
---------------------

For each step of the process described of :ref:`basis-members`, Members check
the mode identifying the action it should take. Based on this flag, it will
either call a builtin function or the appropriate user provided function or
method.

One can inspect the mode set on the member by accesing the matching attribute
as described in the table below. This attribute is a tuple containing two
items. The first item is the flag value matching the member behavior, the
second item depend on the value of the flag.

The behavior of the member can be modified by calling the matching ``set_``
method, to set the flag to a new and provide the additional item matching the
flag value.

For all steps of the getattr, setattr process, you can invoke them separatly
by calling the mathcing ``do_`` method. Among them, |Member.do_full_validate|
is special in that it will run both the validate and post_validate steps on the
provided value.

The following table summarizes the different steps along with the flags and the
aforementionned attributes and methods:

+---------------+----------------+---------------------------------------------------------------+---------------------------+
| Steps         | Mode           | Mode (getter/setter)                                          | Manual running            |
+===============+================+===============================================================+===========================+
| getattr       | |GetAttr|      | |Member.getattr_mode| / |Member.set_getattr_mode|             | |Member.do_getattr|       |
+---------------+----------------+---------------------------------------------------------------+---------------------------+
| post_getattr  | |PostGetAttr|  | |Member.post_getattr_mode| / |Member.set_post_getattr_mode|   | |Member.do_post_getattr|  |
+---------------+----------------+---------------------------------------------------------------+---------------------------+
| setattr       | |SetAttr|      | |Member.setattr_mode| / |Member.set_setattr_mode|             | |Member.do_setattr|       |
+---------------+----------------+---------------------------------------------------------------+---------------------------+
| post_setattr  | |PostSetAttr|  | |Member.post_setattr_mode| / |Member.set_post_setattr_mode|   | |Member.do_post_setattr|  |
+---------------+----------------+---------------------------------------------------------------+---------------------------+
| validate      | |Validate|     | |Member.validate_mode| / |Member.set_validate_mode|           | |Member.do_validate|      |
+---------------+----------------+---------------------------------------------------------------+---------------------------+
| post_validate | |PostValidate| | |Member.post_validate_mode| / |Member.set_post_validate_mode| | |Member.do_post_validate| |
+---------------+----------------+---------------------------------------------------------------+---------------------------+
| default_value | |DefaultValue| | |Member.default_value_mode| / |Member.set_default_value_mode| | |Member.do_default_value| |
+---------------+----------------+---------------------------------------------------------------+---------------------------+

.. note::

    Delattr works on the same model but the flag is not exposed as part of the
    public API. You can still access the mode using |Member.delattr_mode|


Behaviors for custom members
----------------------------

In order to create custom members, you can either subclass |Member| and set
the modes in the ``__init__`` method, or set the modes after instantiating the
member. The modes that can be used in conjunction with custom callable or
methods are listed below and expected signature of the callable or the method
can be directly inferred from the mode. When specifying a method, the second
item of the mode should be the name of the method. In the following, **Object**
always  refers to an |Atom| subclass instance and **Name** to the member name.

- |Getattr|:
    - CallObject_Object
    - CallObject_ObjectName
    - ObjectMethod
    - ObjectMethod_Name
    - MemberMethod_Object

- |PostGetAttr|:
    - ObjectMethod_Value
    - ObjectMethod_NameValue
    - MemberMethod_ObjectValue

- |SetAttr|:
    - CallObject_ObjectValue
    - CallObject_ObjectNameValue
    - ObjectMethod_Value
    - ObjectMethod_NameValue
    - MemberMethod_ObjectValue

- |PostSetAttr|:
    - ObjectMethod_OldNew
    - ObjectMethod_NameOldNew
    - MemberMethod_ObjectOldNew

- |DefaultValue|:
    - CallObject
    - CallObject_Object
    - CallObject_ObjectName
    - ObjectMethod
    - ObjectMethod_Name
    - MemberMethod_Object

- |Validate|:
    - ObjectMethod_OldNew
    - ObjectMethod_NameOldNew
    - MemberMethod_ObjectOldNew

- |PostValidate|:
    - ObjectMethod_OldNew
    - ObjectMethod_NameOldNew
    - MemberMethod_ObjectOldNew

- |GetState|:
    - ObjectMethod_Name
    - MemberMethod_Object

.. note::

    It is recommended to avoid customizing specialized members that may make
    some assumptions regarding the values of the other modes.

Using ``__init_subclass__``
---------------------------

Starting with atom 0.10.0, ``__init_subclass__`` can be meaningfully used to
customize members as illustrated in :ref:`ex-pickling`.

When accessing members in ``__init_subclass__`` one should however be careful
to only modify members that belong to the class being customized. Otherwise
a parent class could see its behavior change which is definitively undesirable.
To avoid this issue |clone_if_needed| can be used to clone a member if it does
not belong to a class and update the class to use the clone. It returns the
original member or its clone.
