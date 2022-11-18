# iMSTK - Interactive Medical Simulation Toolkit
* [About](#about)
  * [Overview](#overview)
  * [Participants](#participants)
  * [Licensing](#licensing)
* [Resources](#resources)
  * [Documentation](#documentation)
  * [Discourse](#discourse)
  * [Issue tracker](#issue-tracker)
* [Prerequisites](#prerequisites)
* [Getting started with iMSTK](#getting-started-with-imstk)
  * [Getting the source code](#1-getting-the-source-code)
  * [Setting up your SSH key](#2-setting-up-your-ssh-key)
  * [Building iMSTK](#3-building-imstk)
    * [On Linux/macOS](#on-linuxmacos)
    * [On Windows](#on-windows)
    * [Phantom Omni Support](#phantom-omni-support)
    * [Haply Inverse3 Support](#haply-support)
    * [VRPN Support](#vrpn-support)
    * [Offscreen Rendering](#offscreen-rendering)
    * [Renderless](#renderless)

## About
### Overview
[iMSTK](https://www.imstk.org/) is a C++ based free & open-source toolkit that aids rapid prototyping of real-time multi-modal surgical simulation scenarios. Surgical simulation scenarios involve algorithms from areas such as haptics, advanced rendering, computational geometry, computational mechanics, virtual reality and parallel computing. iMSTK employs a highly modular and extensible design to enable the use of libraries and codes from these areas in a given application thereby reducing the development time.

### Participants
- [Kitware, Inc.](http://www.kitware.com/)
- [CeMSIM-Rensselaer Polytechnic Institute](http://cemsim.rpi.edu/)
- [University of Central Arkansas](http://sun0.cs.uca.edu/~thalic/virasim.html)

### Funding support:
This project is supported in part by the following grants [9R44OD018334](https://www.sbir.gov/sbirsearch/detail/1032259), [1R44EB019802](https://www.sbir.gov/sbirsearch/detail/1047037), [1R44AR075481](https://projectreporter.nih.gov/project_info_details.cfm?aid=9777225&icde=50531419) , [1R01EB025247](https://projectreporter.nih.gov/project_info_details.cfm?aid=9738646&icde=50531433), [2R44DK115332](https://projectreporter.nih.gov/project_info_details.cfm?aid=9843084&icde=50531443)

Disclaimer: The content is solely the responsibility of the authors and does not necessarily represent the official views of the NIH and its institutes.

### Licensing
iMSTK is licensed under [Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0.txt)

## Resources
### Documentation
Click [here](https://imstk.gitlab.io/) for user documentation.

Click [here](https://imstk.gitlab.io/doxy/index) for doxygen code documentation.

Click [here](https://gitlab.kitware.com/iMSTK/iMSTK/-/blob/master/Docs/CodingGuide.md) for iMSTK coding style

Click [here](https://gitlab.kitware.com/iMSTK/iMSTK/-/blob/master/Docs/Maintainance.md) to view information about iMSTK maintenance tasks

### Dashboard
Click [here](https://open.cdash.org/index.php?project=iMSTK) for nightly build & MR results.

### Discourse
The iMSTK Users and Developers can post questions and discuss relevant topics at the [Discourse](https://discourse.kitware.com/c/imstk).

### Issue-tracker
Designed more specifically for developers, the issue tracker allows developers to list and discuss issues & enhancements:
>https://gitlab.kitware.com/iMSTK/iMSTK/issues

*Assign labels to the issues. The description of each label can be found [HERE](https://gitlab.kitware.com/iMSTK/iMSTK/labels).*

<a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=bug"><span class="label color-label " style="background-color: #cc0033; color: #FFFFFF" title="Report an error at runtime" >bug</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=compilation"><span class="label color-label " style="background-color: #cc0033; color: #FFFFFF" title="Report an error during compilation" >compilation</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=critical"><span class="label color-label " style="background-color: #ff0000; color: #FFFFFF" title="Issue that should require the developers main focus" >critical</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=enhancement"><span class="label color-label " style="background-color: #5cb85c; color: #FFFFFF" title="Suggest an enhancement you believe is needed (new features...)" >enhancement</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=optimization"><span class="label color-label " style="background-color: #5cb85c; color: #FFFFFF" title="Report a slow process and possibly offer ideas to optimize it" >optimization</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=clean+up"><span class="label color-label " style="background-color: #428bca; color: #FFFFFF" title="Suggestions to improve the code style" >clean up</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=refactor"><span class="label color-label " style="background-color: #428bca; color: #FFFFFF" title="Suggest a better way to implement a certain feature" >refactor</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=testcase"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a test or example within the project" >testcase</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=visualization"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a visualization feature" >visualization</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=mechanics"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a mechanics feature" >mechanics</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=device"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a device feature" >device</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=documentation"><span class="label color-label " style="background-color: #f0ad4e; color: #FFFFFF" title="Report an issue/requirement that is related to documentation (code, project...)" >documentation</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=support"><span class="label color-label " style="background-color: #f0ad4e; color: #FFFFFF" title="Report an issue/requirement that is related to support (dashboard, mailing list, website...)" >support</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=discussion"><span class="label color-label " style="background-color: #8e44ad; color: #FFFFFF" title="Start a discussion about a certain topic that requires other users and developers input" >discussion</span></a>

## Prerequisites
* Git
* Git LFS
* Python (if using Uncrustify, default on)
* CMake 3.15 or higher

##### On Linux:

```bash
sudo apt-get install build-essential libgl1-mesa-dev libxt-dev libusb-1.0-0-dev git-lfs
```

## Getting started with iMSTK
### 1. Getting the source code
To be able to contribute back to the iMSTK project, the preferred way is to use [Git] for code version control. You can use the following command in the terminal for Linux/macOS, or in [Git Bash] for Windows.
```sh
git clone https://gitlab.kitware.com/iMSTK/iMSTK.git
```
OR
```sh
git clone git@gitlab.kitware.com:iMSTK/iMSTK.git
```

### 2. Optional: Setting up your SSH key
If cloning with HTTPS, skip this step. If cloning with SSH the build process will check out external dependency sources with the SSH protocol to avoid manually entering credentials during the build process. To allow this, make sure you set up your ssh key in your profile [HERE](https://gitlab.kitware.com/profile/keys). You can find documentation on how to generate and retrieve your public ssh key [HERE](https://gitlab.kitware.com/help/ssh/README).

### 3. Building iMSTK
We use [CMake] to configure the project on every platform. See how to run it [HERE](https://cmake.org/runningcmake/).

* #### On Linux/macOS
  Type the following commands from the same location you cloned the code. This will configure the build in a directory adjacent to the source directory. To easily change some configuration variables like `CMAKE_BUILD_TYPE`, use `ccmake` instead of `cmake`.
  ```sh
  mkdir iMSTK-build
  cd iMSTK-build
  cmake ../iMSTK #/path/to/source/directory
  make -j4 #to build using 4 cores
  ```
  You can also use [Ninja] for a faster build instead of Unix Makefiles. To do so, configure the cmake project with `-GNinja`:
  ```
  cmake -GNinja ../iMSTK
  ninja
  ```
  This will checkout, build and link all iMSTK dependencies. When making changes to iMSTK [base source code](/Base), you can then build from the `Innerbuild` directory.

* #### On Windows
  Run CMake-GUI and follow the directions described [HERE](https://cmake.org/runningcmake/). You will have to choose which version of Visual Studio you'd like to use when configuring the project, make sure to select **Microsoft Visual Studio 2017, 2019 or 2022**. CMake will generate a `iMSTK.sln` solution file for Visual Studio at the top level for what is called the Superbuild. Open this file and build all targets, which will checkout, build and link all iMSTK dependencies. Make sure the build mode is set to `Release` or `RelWithDebInfo`, iMSTK is not very usable in `Debug` mode. The switches IMSTK_BUILD_EXAMPLES, IMSTK_BUILD_TESTING and IMSTK_BUILD_VISUAL_TESTING should be on as well. When making changes to iMSTK [base source code](/Base), you can then build from the `iMSTK.sln` solution file located in the `Innerbuild` directory.
  If you would like to build on multiple cores add /MP[N] to CMAKE_CXX_FLAGS in CMake-GUI, where N is optional representing the number of cores (without N supplied, the build will use as many cores as available on the device).
  If you check out the unit tests or the examples make sure to run `git install lfs` to make sure that `git lfs` is installed.

* #### 3DSystems Touch Support
  To support the [3DSystems Touch (formerly Geomagic Touch or Sensable Phantom Omni)](https://www.3dsystems.com/haptics-devices/touch) haptic device, follow the steps below:
    1. Install the [OpenHaptics] SDK as well as the device drivers:
        - for [Windows](https://support.3dsystems.com/s/article/OpenHaptics-for-Windows-Developer-Edition-v35?language=en_US)
        - for [Linux](https://support.3dsystems.com/s/article/OpenHaptics-for-Linux-Developer-Edition-v34?language=en_US)
    2. Reboot your system.
    3. Configure your CMake project with the variable `iMSTK_USE_OpenHaptics` set to `ON`.
    4. After configuration, the CMake variable `OPENHAPTICS_ROOT_DIR` should be set to the OpenHaptics path on your system.
  
* #### Haply Support
  To support the [Haply Inverse3](https://www.haply.co) haptic device, follow the steps below:
    1. Acquire the C++ Haply Hardware API from [here](https://talk.haply.co/t/release-of-the-version-0-0-5-of-the-c-api/19/4).
    2. Add the Haply Hardware API to your system path with variable name: Haply_SDK_BASE.
    3. Configure your CMake project with the variable `iMSTK_USE_HAPLY` set to `ON`.

  Please note the Haply Hardware API is in early stages of development and will be deprecated & changed in the future.

* #### VRPN Support

  The `VRPNDeviceModule` enables access to a large number devices supported by VRPN. The `VRPNDeviceModule` expects a `vrpn_server` to be running. The iMSTK superbuild builds and
  installs a server with some default settings but if you want to configure a specific server
  it might be easier to separately build a server. The file `CMake\External\External_VRPN.cmake`
  shows how to pass configuration into VRPN in case you want to modify the modules enabled and
  disabled by the build

  The `vrpn.cfg` that is installed by default doesn't have any devices enabled, before use you 
  will need to uncomment the devices that you would like to use. If you use the one inside the 
  iMSTK install directory please note that it will be overwritten every time the superbuild is 
  run. 

  Currently iMSTK supports VRPN `Analog`, `Button` and `Tracker` devices. Future support will
  depend on user demand.

* #### Offscreen Rendering
  To render without the usage of a GPU or without the usage of a screen on linux (or WSL), iMSTK's VTK renderer may be built with OSMesa.
    1. Install osmesa libraries via:
  ```bash
  sudo apt install mesa-common-dev libosmesa6-dev libglu1-mesa-dev`
  ```
    2. Set `iMSTK_USE_VTK_OSMESA` to `ON`
    3. Proceed to build iMSTK

* #### Renderless
  To build iMSTK without any rendering & completely as a physics backend, use:
    1. Set `iMSTK_USE_RENDERING_VTK` to `OFF`
    2. Set `iMSTK_BUILD_VISUAL_TESTING` to `OFF`

---
[NIH-OD]: <https://www.nih.gov/about-nih/what-we-do/nih-almanac/office-director-nih>
[NIH-NIBIB]: <https://www.nibib.nih.gov/>
[Rensselaer Polytechnic Institute]: <www.rpi.edu>
[Kitware, Inc.]: <www.kitware.com>
[Git Bash]: <https://git-for-windows.github.io/>
[Git]: <https://git-scm.com>
[CMake]: <https://cmake.org>
[Ninja]: <https://ninja-build.org/>
[OpenHaptics]: <https://www.3dsystems.com/haptics-devices/openhaptics>