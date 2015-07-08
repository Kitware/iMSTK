.. role:: cxx(code)
   :language: c++

.. role:: arg(code)
   :language: sh

.. _simmedtk-administration:

**********************
Administering SimMedTK
**********************

Previous sections covered the concepts and tools for using SimMedTK.
This section is for system administrators who wish to make SimMedTK
available to users

* via command-line utilities for end users of SimMedTK, and/or
* as a library for people developing SimMedTK-based applications.

End-user tool installation
==========================

This type of installation should be as simple as downloading a
binary package of SimMedTK and clicking install.

.. todo:: Expand on details of installation and configuration.

Developer installation
======================

In addition to the binary installer, there should also be a development
package that contains header and configuration files needed to build
C++ applications using SimMedTK. Install this the same way you installed
the binary above.

You can also download the source code from the git repostory and
follow the instructions for building and installing SimMedTK in the
toplevel :file:`ReadMe.mkd` file.

.. todo:: Expand on details of installation and configuration.

Configuration of SimMedTK
=========================

Some components of SimMedTK will not be available unless
they are properly configured.
One example is access to haptic or virtual reality display
devices.

.. todo::

   Expand on details of configuration here, especially things only
   sysadmins would need to customize as opposed to end users.

Search paths
------------

.. todo::

   Explain how SimMedTK searches for simulation inputs.

The default locations that SimMedTK searches for these
worker files varies by operating system:

Linux
    SimMedTK searches the current working directory of the
    process, followed by the :file:`var/simmedtk` subdirectory
    of the toplevel installation directory.
    For example, if SimMedTK is installed into :file:`/usr`
    then it will search :file:`/usr/var/simmedtk`.

    If the :cxx:`SimMedTK_CONFIG_DIR` environment variable is set
    to a valid path, then it is searched as well.

Mac OS X
    SimMedTK searches the current working directory of the
    process, followed by the :file:`var/simmedtk` subdirectory
    of the toplevel installation directory if SimMedTK is not part of a bundle.
    For example, if SimMedTK is installed into :file:`/usr`
    then it will search :file:`/usr/var/simmedtk/workers`.

    If an application built with SimMedTK is part of a bundle (such as an app),
    then SimMedTK will search the :file:`Contents/Resources` directory
    of the bundle.

    If the :cxx:`SimMedTK_CONFIG_DIR` environment variable is set
    to a valid path, then it is searched as well.

Windows
    SimMedTK searches the current working directory of the process
    followed by the directory containing the process executable
    (when provided to SimMedTK by the application).

    If the :cxx:`SimMedTK_CONFIG_DIR` environment variable is set
    to a valid path, then it is searched as well.

Creating simulation scenes
--------------------------

Other portions of this manual have covered how to create a
custom simulation by writing C++ code.
However, in some cases, it is sufficient to just create new
or adapt existing configuration files and use the simulation
program that comes with SimMedTK.

The simulation program that comes with SimMedTK is a
command-line utility named :file:`vegaFemExample`.
You can run

.. code:: sh

  vegaFemExample -help

to obtain reference information on the command-line arguments.

.. todo:: Describe changing input decks in more detail and verify the notes above.
