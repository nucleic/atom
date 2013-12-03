#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
""" Generate the Example Documentation for the Atom Examples

Run as part of the documentation build script.
Look for example atom files with the line:

<< autodoc-me >>

"""
from __future__ import print_function
import os
import subprocess


def generate_example_doc(docs_path, script_path):
    """ Generate an RST for an example file.

    Parameters
    ----------
    docs_path : str
         Full path to atom/docs/source/examples
    script_path : str
         Full path to the example atom file
    """
    script_file_name  = os.path.basename(script_path)
    script_name = script_file_name[:script_file_name.find('.')]
    print('generating doc for %s' % script_name)
    
    script_title = script_name.replace('_', ' ').title()
    rst_path = os.path.join(
        docs_path, 'ex_' + script_name + '.rst')
    relative_script_path = script_path[
        script_path.find('examples'):].replace('\\', '/')

    with open(os.path.join(script_path)) as fid:
        script_text = fid.read()

    docstring = script_text[script_text.find('"""') + 3:]
    docstring = docstring[: docstring.find('"""')]
    docstring = docstring.replace('<< autodoc-me >>\n', '').strip()

    rst_template = """
{0} Example
===============================================================================

:download:`{1} <../../../{2}>`

::

    {3}

::

 $ python {1}

.. literalinclude:: ../../../{2}
    :language: python

""".format(script_title, script_file_name, relative_script_path,
           docstring.replace('\n', '\n    '))

    output = subprocess.check_output('python %s' % script_file_name,
                                     cwd=os.path.dirname(script_path))
    if output:
        rst_template += """
Output:

.. code-block:: python
      
    {0}""".format(output.replace('\n', '\n    '))

    with open(rst_path, 'wb') as fid:
        fid.write(rst_template.lstrip())


def main():
    """ Generate documentation for all atom examples requesting autodoc.

    Looks in atom/examples for all atom files, then looks for the line:
    << auto-doc >>

    If the line appears in the script, generate an RST for the example.
    """
    docs_path = os.path.dirname(__file__)
    base_path = '../../../examples'
    base_path = os.path.realpath(os.path.join(docs_path, base_path))

    for dirname, dirnames, filenames in os.walk(base_path):
        files = [os.path.join(dirname, f)
                 for f in filenames if f.endswith('.py')]

        for fname in files:
            with open(fname, 'rb') as fid:
                data = fid.read()
            if '<< autodoc-me >>' in data.splitlines():
                try:
                    generate_example_doc(docs_path, fname)
                except KeyboardInterrupt:
                    return


if __name__ == '__main__':
    main()
