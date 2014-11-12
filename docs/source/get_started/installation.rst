.. _installation:

============
Installation
============

Installing Atom is a straight forward process. It requires few external
dependencies, and those which are required are easily installed, with most
projects providing binaries or a simple Python setup script. The sections
below describe how to install Atom and all of its dependencies from scratch,
starting with the installation of a Python runtime. The instructions assume
that the user's system has a C++ compiler and the `Git`_ command line tools
installed and available on the system path.

.. _Git: http://git-scm.com


.. topic:: The Easy Way

    If installing and building Atom and its dependencies from scratch is not
    appealing, the free (and unaffiliated) `Anaconda`_ Python distribution
    provides a complete Python environment which comes with a reasonably
    recent version of Atom and a host of other useful packages.
    
    If you have a working C++ compiler, you can install using pip::

    $ pip install atom

.. _Anaconda: https://store.continuum.io/cshop/anaconda


.. topic:: Supported Platforms

    Atom is known to run on Windows, OSX, and Linux; and compiles cleanly
    with MSVC, Clang, GCC, and MinGW. However, primary development of the
    framework occurs on Windows (7 and 8), so some quirks and bugs may be
    present on the other platforms. If you encounter a bug, please report
    it on the `Issue Tracker`_.

.. _Issue Tracker: http://github.com/nucleic/atom/issues


`Python`_
---------

Atom is a Python framework and requires a supported Python runtime. Atom
currently supports **Python 2.6** and **Python 2.7**. Python 3.x support may
be added in the future, but is not currently a high priority item.

The most recent Python 2.x series releases are available on the
`Python Downloads`_ pages. Installers are available for Windows and OSX.
Linux users should install Python using their OS package manager.

.. _Python: http://python.org
.. _Python Downloads: http://python.org/download


`Setuptools`_
-------------

Setuptools is a Python package which makes installing other Python packages a
breeze. Some of the installation instructions below assume that Setuptools has
been installed in the target Python environment. Follow the relevant
`Setuptools Install`_ instructions for adding the package to your system.

.. _Setuptools: http://pythonhosted.org/setuptools
.. _Setuptools Install: https://pypi.python.org/pypi/setuptools/1.1.6


`Atom`_
-------

The last item on the list is Atom itself, and it can be installed with just
a few commands::

    C:\> git clone https://github.com/nucleic/atom.git
    C:\> cd atom
    C:\> python setup.py install

.. _Atom: https://github.com/nucleic/atom
