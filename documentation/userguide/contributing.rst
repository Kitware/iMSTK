************************
Contributing to SimMedTK
************************

.. role:: cxx(code)
   :language: c++

.. role:: shell(code)
   :language: sh

.. contents::

The first step to contributing to SimMedTK is to obtain the source code and build it.
The top-level ReadMe.mkd file in the source code includes instructions for building SimMedTK.
The rest of this section discusses how the source and documentation are organized
and provides guidelines for how to match the SimMedTK style.

Source code organization
========================

SimMedTK currently keeps its header and implementation files in parallel directory structures.
Implementation files live in :file:`src` while header files live in :file:`include`.
Inside each of these directories are a number of subdirectories corresponding to
libraries that encapsulate difference components of SimMedTK.
With that in mind:

* include — This directory contains all of the header files for SimMedTK libraries
* src — This directory contains all of the source code for SimMedTK libraries and unit tests.
  The directories correspond to library names.

  * smCore — a library for classes used throughout SimMedTK
  * smMesh — scene objects that participate in the physics simulation
  * smTools — models of surgical instruments
  * smCollision — implementations of collision *detection* algorithms
  * smContactHandling — implementations of collision *response* algorithms
  * smSimulators — algorithms that perform timestepping
  * smGeometry — scene objects such as planes and polygonal models that do not participate in the physics simulation
  * smRendering — source for :ref:`sm-rendering-sys` rendering code
  * smRenderDelegates — library containing classes that render meshes and other scene objects
  * smEvent — user interface event handlers
  * smExternal — third-party code that is not part of a library
  * smExternalDevices — interfaces to haptics devices

* examples — demonstration applications that use SimMedTK libraries
* documentation — user's guide documentation and configuration for the in-source documentation
* CMake — scripts to aid in configuring, installing, and packaging SimMedTK

Inside some :file:`src/`, subdirectories, there are :file:`UnitTests/` directories that
contain Bandit_-based unit tests.

.. _Bandit: http://banditcpp.org/

Code style
==========

* No tabs or trailing whitespace are allowed.
* Indent blocks by 2 spaces.
* Class names should be camel case, starting with an uppercase.
* Class member variables should start with :cxx:`m_` or :cxx:`s_` for per-instance or class-static variables, respectively.
* Class methods should be camel case starting with a lowercase character (except acronyms which should be all-uppercase).
* Use shared pointers and a static :cxx:`create()` method for classes that own significant storage or must be passed by
  reference to their superclass.

Submitting and reviewing changes to SimMedTK
============================================

Once you have made a change to SimMedTK that you would like to contribute,
you should submit a merge request to the canonical SimMedTK gitlab repository.
When you submit the merge request, please use the following checklist to
make reviewing the change as painless as possible:

* You must give
  (a) **buildbot**,
  (b) any potential (human) **reviewers**, and
  (c) **owners** of dashboard buildslaves
  read access to your SimMedTK repository.
  This is mandatory because Gitlab_ enforces permissions even on branches you have
  submitted for review.
  If reviewers do not have permission, they will not approve your branch for merging.
  If buildbot does not have permission, tests cannot be queued.
  If dashboard machine owners do not have permission, then tests will be queued but will
  fail because the build slaves will not be able to check out your revisions.
  (Yes, dashboard machines currently use their human owner's SSH keys to fetch revisions.)

* Every commit in the merge request **must build and run** properly so that :shell:`git bisect`
  can be used to track down issues.

* The merge request should have as few commits as possible to make reviewing as simple as possible.
  Use :shell:`git rebase -i` to *squash* commits.
  This does not mean that a meaningful sequence of commits must be squashed, but if your commits
  are not a clear, logical sequence you should consider squashing them.

* If you are assigned to review a merge request,

    * submit the branch for testing by adding the comment "@buildbot test" if
      tests have not already been queued (and you think the request should be considered).
      Submitting a request for testing should not be interpreted as approval of the request.
    * use a "-1" comment to veto the merge request (and explain why)
    * use a "+1" comment to indicate you've looked at the source changes and approve
    * use a "+2" comment to indicate you've fetched the source, built, and tested it yourself
    * do **not** "accept" the merge request (i.e., merge the branch into the master SimMedTK repository)
      unless you are certain that the submitter does not have the authority to perform the merge.
      The submitter should perform the merge whenever possible;
      whoever performs the merge is responsible for monitoring the dashboard for failing tests.
      Just because a merge request builds and passes tests does not mean that the master
      branch will also build and pass tests.
      (Failures can be introduced by intervening commits in other merges or by additional
      platform tests performed by dashboards that only test the master branch.)

* If you have submitted a request, you may "accept" it (i.e., merge the branch in to the master
  SimMedTK repository) when
    * your request has a "+2" or two "+1" comments and no unresolved "-1" comments.
      (Resolving a "-1" must include the submitting reviewer agreeing to remove the object
      or another reviewer explicitly overriding the objection in a comment.)
      Submitting a request for testing should not be interpreted as approval of the request.
    * dashboards run on the request (queued by you or a reviewer) **all** build and pass tests.
      Branches that introduce new test failures should never be merged.
   Whoever performs the merge is responsible for monitoring the dashboard for failing tests.
   Just because a merge request builds and passes tests does not mean that the master
   branch will also build and pass tests.
   (Failures can be introduced by intervening commits in other merges or by additional
   platform tests performed by dashboards that only test the master branch.)

Using SimMedTK from another project
===================================

.. todo::

  SimMedTK does not currently export a SimMedTKConfig.cmake file like it should.

Extending SimMedTK
==================

See the tutorials for in-depth guides on how to extend SimMedTK
in certain obvious directions,

* Preparing a scene for a simulation
* Writing a new render delegate.
* Writing an exporter to support a new solver's input format.

Documentation style
===================

There are two types of documentation in SimMedTK:
Doxygen_ documentation written as comments in C++ code and
Sphinx_ documentation written in reStructuredText_ files (and optionally Python documentation strings).
The former is used to create reference documentation; the latter is used for the user's guide and tutorials.

The following rules apply to writing documentation:

* Header files should contain the Doxygen documentation for the class as a whole plus any enums declared outside classes, however:
* Implementation files should contain the Doxygen documentation for class methods.
  This keeps the documentation next to the implementation (making it easier to keep up-to-date).
  It also makes the headers easier to read.
* If a class provides high-level functionality, consider writing some user-guide-style documentation
  in the User's Guide (in :file:`doc/userguide.rst`) or a tutorial (in :file:`doc/tutorials/`).
  Tutorials should include a working example that is run as a CTest test.
  The code in the example should be referenced indirectly by the tutorial so that
  the the exact code that is tested appears as the text of the tutorial.
* In reStructuredText documents, you should use the doxylinks_ module to link to
  the Doxygen documentation *when appropriate*.
  Examples:
  ``:sm:`smCoreClass``` produces this link: :sm:`smCoreClass` while the
  ``:sm:`CoreClass <smCoreClass>``` variant can produce
  links (:sm:`CoreClass <smCoreClass>` in this case) whose text varies from the classname
  or whose classnames are ambiguous because of namespaces.
  The leading ``:sm:`` names the tag file holding the class and function definitions;
  other third-party-library tag files may be added in the future.

  You will be tempted to make every word that is a classname into a Doxygen link; do not do this.
  Instead, provide a Doxygen link at the first occurrence of the classname in a topic's
  discussion — or at most in a few key places. Otherwise the documentation becomes difficult to read
  due to conflicting text styles.
* In reStructuredText, when you wish to show code in-line but it is inappropriate to link to Doxygen documentation,
  use the ``:cxx:`` role for C++ (e.g., :cxx:`if (foo)`), the ``:file:`` role for paths to files (e.g., :file:`doc/index.rst`), and so on.
  See the `documentation for roles in reStructuredText`_ for more information.
* Note that the user's guide and tutorials are both included in the top-level :file:`doc/index.rst` file
  parsed by Sphinx.
  Several extensions to Sphinx are used and these are configured in :file:`doc/conf.py`.

To get started documenting your code, you should at least have doxygen_ and graphviz_ installed.
These are available using Homebrew_ on Mac OS X, your Linux distribution's package manager, or by binary
installer from the source maintainer on Windows.

Additionally there are a number of Python packages that provide Sphinx, docutils, and other packages required
to generate the user's guide.
These packages can all be installed with pip:

.. highlight:: sh
.. code-block:: sh

  # The basic utilities for processing the user's guide:
  sudo pip install docutils
  sudo pip install Sphinx
  # For linking to external Doxygen docs:
  sudo pip install sphinxcontrib-doxylink
  # For creating inline class docs from Doxygen XML:
  sudo pip install breathe
  # For the default theme:
  sudo pip install sphinx-rtd-theme
  # For syntax highlighting:
  sudo pip install Pygments
  # For activity diagrams:
  sudo pip install sphinxcontrib-actdiag
  # For image handling:
  sudo pip install pillow
  # For coloring html:
  sudo pip install webcolor

If you are unfamiliar with the documentation packages here, see these links for examples of their use
(or use SimMedTK by example):

* `Sphinx Table of Contents <http://sphinx-doc.org/contents.html>`_
* `Sphinx conf.py configuration <http://sphinx-doc.org/config.html>`_
* `reStructuredText primer <http://sphinx-doc.org/rest.html>`_
* `Doxygen commands <http://www.stack.nl/~dimitri/doxygen/manual/index.html>`_


.. _doxygen: http://doxygen.org/
.. _doxylinks: https://pypi.python.org/pypi/sphinxcontrib-doxylink
.. _Gitlab: https://gitlab.com/
.. _graphviz: http://graphviz.org/
.. _Homebrew: http://brew.sh/
.. _Sphinx: http://sphinx-doc.org/
.. _reStructuredText: http://docutils.sourceforge.net/rst.html
.. _documentation for roles in reStructuredText: http://sphinx-doc.org/markup/inline.html

To-do list
==========

Finally, if you are looking for a way to contribute,
helping with the documentation would be great.
A list of incomplete documentation (or incomplete features)
is below.
You can also look on the SimMedTK issue tracker for things to do.

.. todolist::
