=====================
SimMedTK User's Guide
=====================

The Simulations for Medicine Tool Kit, or SimMedTK, is a framework
to help you create interactive applications that simulate mechanical
(and other) aspects of biological systems.

For instructions on obtaining, building, and installing
SMTK, clone the repository:

.. code:: sh

   git clone https://gitlab.kitware.com/SimMedTK/SimMedTK.git

and follow the instructions in the :file:`ReadMe.mkd` file
in the top-level source directory.
The rest of this user's guide assumes you have built
and installed SMTK according to these instructions.

.. todo::

   Add high-level discussion of the components of the toolkit
   and how they fit together (before the "administration.rst"
   line below).

.. toctree::
   :maxdepth: 4

   administration.rst
   contributing.rst

.. role:: cxx(code)
   :language: c++
