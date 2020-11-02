Atom Release Notes
==================

0.6.0 - unreleased
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
