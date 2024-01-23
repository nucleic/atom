Atom Release Notes
==================

0.10.4 - 23/01/2024
-------------------

- allow unions in TypeVar bound PR #207

0.10.3 - 04/10/2023
-------------------

- fix a an issue when using add_member to override an existing member PR #204

  Once again a huge thanks to @frmdstryr for the report and fix

0.10.2 - 02/10/2023
-------------------

- fix a memory leak caused by Dict, Defaultdict and Set members #202

  A huge thanks to @frmdstryr for the report and fix

0.10.1 - 11/09/2023
-------------------

- add support for Python 3.12 PR #200

0.10.0 - 05/05/2023
-------------------

- AtomMeta: create the class only once all members have been customized PR #510
  This allows to use ``__init_subclass__`` since previously the ``__atom_members__``
  was not set when ``__init_subclass__`` was called
- illustrate the use of ``__init_subclass__`` to customize pickling PR #510
- refactor the metaclass to make it easier to reason about PR #510
- add a ``DefaultDict`` member which enforce the value to be a dictionary and
  use a factory for missing keys. The factory is set at the member level and
  a normal dict can be provided as value. When possible the factory is deduced
  from the member used for validation.
- infer the use of ``DefaultDict`` from a ``collections.defaultdict`` annotation

0.9.1 - 13/03/2023
------------------

- fix generation of Value member from union containing object/Any PR #198
- fix setdefault method of atomdict to return the actually stored item PR #197

0.9.0 - 21/02/2023
------------------

- fix the generation of Subclass from type annotations PR #193
- move getstate and setstate implementation to C and allow to customize the condition
  under which a member value is pickled. PR #182
  Customization is used to only pickle member value whose state can be restored
  (for example Constant is not pickled). Whether a member is pickled can be customized
  using a method prefixed with ``_getstate_`` on an Atom subclass.
- use the Typed member when a type annotation resolve to a single type whose
  metaclass does not implement a custom ``__instancecheck__``  PR #185
- generate Tuple member for tuple[] annotations.
  The content of the tuple is typed check only if it is a 1-tuple or variable-length
  tuple. PR #184
- fix the resolution order of members in the presence of multiple inheritance
  with a common base class PR #165 #168

  Due to the above changes, adding a member after the class definition requires
  more work than before. As a consequence a new helper function ``add_member`` has
  been added.

0.8.2 - 18/10/2022
------------------

- fix handling of Union in annotation based class definition PR #177

0.8.1 - 13/06/2022
------------------

- add support for Python 3.11 PR #166

0.8.0 - 30/03/2022
------------------

This release introduced a new way to declare atom object based on type annotations.

- emit warnings when a magic method points to an undefined member PR #139 #149
- use isort, black and flake8 to ensure a consistent coding style PR #141
- base the version number on the most recent git tag using setuptools-scm PR #141
- make the setup compatible with PEP 517 and PEP 621 compliant PR #141 #162
  Pip should be used for development install in place of directly calling
  setup.py
- fix handling of _SpecialGenericAlias (typing.Sequence, ...) when used inside
  an Instance member PR #151
- add a ChangeDict TypedDict to help annotate observers PR #133
- add several keyword argument to the AtomMeta metaclass  PR #133

   - enable_weakrefs: allow to have weak references to an Atom object without
     having to add the slot by hand. False by default.
   - use_annotations: generate members from type annotations. Str-like annotations
     are not supported but allowed when an actual member is provided as default value.
     True by default.
   - type_containers: in conjunction with use_annotations determine to what depth
     to type the content of a container. The default is 1 meaning that list[int]
     will use List(Int()) but list[list[int]] will use List(List()).
- allow specifying which change events are emitted when adding static observers PR #155


0.7.0 - 21/11/2021
------------------

This release introduces several minor backward incompatibilities which are detailed
below. Those are expected to impact only a small minority of users since they make
behaviors more in line with users expectations in most cases or can be easily addressed.

- allow to use any subscribed type in Typed and Instance. Optional and Union are
  analyzed to extract the tuple of types to validate. The presence of NoneType in
  the tuple will make the member optional. For container types (list, dict, set, etc)
  the content types is not used beyond static type validation. PR #136 #140
  Note however that this usage of Optional and Union breaks static analyzer currently,
  while things deriving from an actual type will work as expected (List[int], list[int],
  Iterable[int])
- make the factory argument of Typed, Instance and their forwarded version
  keyword only. PR #123
- add a keyword-only argument to Typed, Instance and their forwarded
  version: ``optional``. When set to False, this will cause those members to
  reject None as a valid value. The default value, None, will resolve to True
  if there is no provided way to build a default value. PR #123 # 131
  This is backward incompatible since previously None was always a valid value.
- the Instance and Typed variants of the Validate enum have been renamed to
  OptionalInstance, OptionalTyped and new Instance and Typed variant describing
  the validation behavior for the member with optional=False have been added. PR #123
- consistently use Instance to wrap types passed to containers. PR #123
  For containers, Instance members used for validation are created with
  optional=False by default. This is backward incompatible since None was always
  a valid value previously.
- add strict argument to FloatRange. PR #124
- allow to specify the type of ReadOnly and Constant. PR #128
  The validation is done using the Instance validator. The change for ReadOnly
  is backward incompatible since the type or tuple of type is the first argument
  in place of the default value. Specifying the default value by keyword is both
  forward and backward compatible.
- use python stdlib IntEnum instead of the custom one in atom  PR #122
- remove the custom atom.IntEnum  PR #122
- add and distribute type hints PR #122 #132
  This allows static type checkers to resolve the values behind a member.
- drop official support for Python 3.6 and add support for Python 3.10


0.6.0 - 02/11/2020
------------------
- remove deprecated members Long and Unicode PR #108
- add support for Python 3.9 PR #108


0.5.2 - 04/07/2020
------------------
- make comparison used in C safe (fix bug introduced in 0.5.0) PR #107


0.5.1 - 03/06/2020
------------------
- fix bug in using atomlist from C PR #102
- clarify Unicode deprecation warning PR #102


0.5.0 - 26/03/2020
------------------
- drop Python 2 support PR #83
- use cppy to write the c++ extension PR #83
- add c++ implememtation of AtomDict PR #93
- add a Set member PR #93


0.4.3 - 18/02/2019
------------------
- improve validation error messages for Instance and Subclass PR #91
- improve validation of validation mode for Instance and Subclass PR #91


0.4.2 - 28/01/2019
------------------
- ensure cached_property are always read-only #84
- improve test coverage #84
- properly implement traverse and clear for atomlist #84 Closes #69
- allow to initialize sortedmap using a dict or an iterable of pairs #84
- fix sortedmap repr #84
- make sortedmap iterable #84
- fix a segfault in SetAttr.ObjectCall_ObjectNameValue #84
- make the clone method of members more homogeneous for container members #84
- add support for Python 3.7 which introduced FAST_CALL protocol for list
  methods #81
- proper size check when creating a list #79
- drop dependency of future that was taking a large time to import #78


0.4.1 - 28/01/2018
------------------
- add a fall-back to type name/pointer comparison on Python 3. Otherwise
  sortedmap can fail for non homogeneously typed keys. #77


0.4.0 - 11/10/2017
------------------
- Python 3 support. _c063e523dd9_

.. _c063e523dd9: https://github.com/nucleic/atom/commit/c063e523dd90919b3d22eac5d49c7e4d7d595039


0.3.10 - 10/28/2014
-------------------
- Fix a bug in resolution of Forward* members. _92244cf1e75_

.. _92244cf1e75: https://github.com/nucleic/atom/commit/92244cf1e75fb81cdfeb5cc498d0b89d0f7cea66

0.3.9 - 04/28/2014
------------------
- Move tests out of main source tree.

0.3.8 - 02/20/2014
------------------
- Move Property behaviors to C++.
- Various maintenance related changes.

0.3.7 - 02/11/2014
------------------
- Add a Subclass member type. a1261b94251_
- Fix a bug with in multiple member assignment. 65cb312e2d7_

.. _a1261b94251: https://github.com/nucleic/atom/commit/a1261b9425196a50fd9b2642e491f7a0cf4f2397
.. _65cb312e2d7: https://github.com/nucleic/atom/commit/65cb312e2d7417a18baaf2816f84b08e483c40bd

0.3.6 - 01/13/2014
------------------
- Fix a subtle bug with multiple inheritance. 82aa3c99270_

.. _82aa3c99270: https://github.com/nucleic/atom/commit/82aa3c992705fa7d9ce45cba1f9b43f4af862ca0

0.3.5 - 11/25/2013
------------------
- Added FloatRange member. ef05758c50e_
- Added 'strict' keyword to Str. 8bda51cfee9_
- Added a 'tag' method to Member for setting metadata. 340adbbf5a9_
- Allow unicode string as observer topics. 441ff55ba73_
- Added a 'strict' keyword to Int. 2ca8b542e8b_
- Added a slew of examples.
- Minor bug fixes and cleanup.

Special thanks to Steven Silvester (@blink1073) for all of his contributions to this release.

.. _ef05758c50e: https://github.com/nucleic/atom/commit/ef05758c50e256074501081dd17d151fd5f906a9
.. _8bda51cfee9: https://github.com/nucleic/atom/commit/8bda51cfee995b32e678dd2cd7bc0b3801e3ad97
.. _340adbbf5a9: https://github.com/nucleic/atom/commit/340adbbf5a9df8913303ab587f45d172254fd862
.. _441ff55ba73: https://github.com/nucleic/atom/commit/441ff55ba739c428b0f6473ed277df961a154761
.. _2ca8b542e8b: https://github.com/nucleic/atom/commit/2ca8b542e8bda067ea1708548cd36281d2941b62

0.3.4 - 10/10/2013
------------------
- Add more flexibility for unobserving an object. f4ac152ffdf_

.. _f4ac152ffdf: https://github.com/nucleic/atom/commit/f4ac152ffdf11a86b30e61d07caa1f718ff77dee

0.3.3 - 09/20/2013
------------------
- Add support for static callable observers. bc630777965_
- Add support for single dotted extended observers. f6a33ee4d57_
- Make atomref a singleton per Atom instance. b793dec1336_

.. _bc630777965: https://github.com/nucleic/atom/commit/bc63077796559b81a7565a20fe4d3299d5b5b6b0
.. _f6a33ee4d57: https://github.com/nucleic/atom/commit/f6a33ee4d5797d7b5659ef5007e84941bf9de54a
.. _b793dec1336: https://github.com/nucleic/atom/commit/b793dec133608c26ac277d4e959b039371e9569e

0.3.2 - 07/20/2013
------------------
- Use Atom instance methods as observers without requiring weakrefs. 31df89b7e8_

.. _31df89b7e8: https://github.com/nucleic/atom/commit/31df89b7e8aa64319e83ad6c8b5012bdeec43a09

0.3.1 - 07/03/2013
------------------
- Bugfix for typed ContainerList. e4f96706ff_
- Make atomlist subclass friendly. 21396e8d1e_
- Add an integer enum class. d2e80e3231_

.. _e4f96706ff: https://github.com/nucleic/atom/commit/e4f96706ff166e107d90376cb88205a51f8db174
.. _21396e8d1e: https://github.com/nucleic/atom/commit/21396e8d1e489556287e12dd9b47434d1589264f
.. _d2e80e3231: https://github.com/nucleic/atom/commit/d2e80e323190b698296a1f21a3837e21f40cbd33

0.3.0 - 05/17/2013
------------------
- First version with release notes.
- Add the ability to monitor the lifetime of an Atom object without weakrefs. 7596aa1b48_
- Move the implemenations for List and ContainerList into C++. 7596aa1b48_
- Add support for pickling Atom objects. 7596aa1b48_

.. _7596aa1b48: https://github.com/nucleic/atom/commit/7596aa1b4884f67ab8266c340e9e5d24c0d15f3b
