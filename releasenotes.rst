Atom Release Notes
==================

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
