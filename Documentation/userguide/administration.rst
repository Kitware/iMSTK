.. role:: cxx(code)
   :language: c++

.. role:: arg(code)
   :language: sh

.. _imstk-administration:

**********************
Administering iMSTK
**********************

Previous sections covered the concepts and tools for using iMSTK.
This section is for system administrators who wish to make iMSTK
available to users

* via a desktop application for end users of iMSTK,
* via command-line utilities for experts preparing simulation scenes, and/or
* as a library for people developing iMSTK-based applications.

End-user tool installation
==========================

This type of installation should be as simple as downloading a
binary package of iMSTK and clicking install.

.. todo:: Expand on details of installation and configuration.

Developer installation
======================

In addition to the binary installer, there should also be a development
package that contains header and configuration files needed to build
C++ applications using iMSTK. Install this the same way you installed
the binary above.

You can also download the source code from the git repostory and
follow the instructions for building and installing iMSTK in the
toplevel :file:`ReadMe.mkd` file.

.. todo:: Expand on details of installation and configuration.

Configuration of iMSTK
=========================

Some components of iMSTK will not be available unless
they are properly configured.
One example is access to haptic or virtual reality display
devices.

.. todo::

   Expand on details of configuration here, especially things only
   sysadmins would need to customize as opposed to end users.

Search paths
------------

.. todo::

   Explain how iMSTK searches for simulation inputs.

The default locations that iMSTK searches for these
worker files varies by operating system:

Linux
    iMSTK searches the current working directory of the
    process, followed by the :file:`var/imstk` subdirectory
    of the toplevel installation directory.
    For example, if iMSTK is installed into :file:`/usr`
    then it will search :file:`/usr/var/imstk`.

    If the :cxx:`iMSTK_CONFIG_DIR` environment variable is set
    to a valid path, then it is searched as well.

Mac OS X
    iMSTK searches the current working directory of the
    process, followed by the :file:`var/imstk` subdirectory
    of the toplevel installation directory if iMSTK is not part of a bundle.
    For example, if iMSTK is installed into :file:`/usr`
    then it will search :file:`/usr/var/imstk/workers`.

    If an application built with iMSTK is part of a bundle (such as an app),
    then iMSTK will search the :file:`Contents/Resources` directory
    of the bundle.

    If the :cxx:`iMSTK_CONFIG_DIR` environment variable is set
    to a valid path, then it is searched as well.

Windows
    iMSTK searches the current working directory of the process
    followed by the directory containing the process executable
    (when provided to iMSTK by the application).

    If the :cxx:`iMSTK_CONFIG_DIR` environment variable is set
    to a valid path, then it is searched as well.

Creating simulation scenes
--------------------------

Other portions of this manual have covered how to create a
custom simulation by writing C++ code.
However, in some cases, it is sufficient to just create new
or adapt existing configuration files and use the simulation
program that comes with iMSTK.

The simulation program that comes with iMSTK is a
command-line utility named :file:`vegaFemExample`.
You can run

.. code:: sh

  vegaFemExample -help

to obtain reference information on the command-line arguments.

.. todo:: Describe changing input decks in more detail and verify the notes above.
