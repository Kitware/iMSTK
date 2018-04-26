# iMSTK - Interactive Medical Simulation Toolkit
* [About](#about)
 * [Overview](#overview)
 * [Participants](#participants)
 * [Licensing](#licensing)
* [Resources](#resources)
 * [Documentation](#documentation)
 * [Mailing-lists](#mailing-lists)
 * [Issue tracker](#issue-tracker)
* [Getting started with iMSTK](#getting-started-with-imstk)
 * [Getting the source code](#1-getting-the-source-code)
 * [Setting up your SSH key](#2-setting-up-your-ssh-key)
 * [Building iMSTK](#3-building-imstk)
* [Using iMSTK in your application](#using-imstk-in-your-application)


## About
### Overview
iMSTK is a C++ based free & open-source toolkit that aids rapid prototyping of real-time multi-modal surgical simulation scenarios. Surgical simulation scenarios involve algorithms from diverse areas such as haptics, advanced rendering, computational geometry, computational mechanics, virtual reality and parallel computing. iMSTK employs a highly modular and extensible design to enable the use of libraries and codes from these areas in a given application thereby reducing the development time.

### Participants
Supported by:
- [NIH-OD] SBIR award [9R44OD018334](https://www.sbir.gov/sbirsearch/detail/1032259)
- [NIH-NIBIB] SBIR award [1R44EB019802-01A1](https://www.sbir.gov/sbirsearch/detail/1047037)

Developed at:
- [Kitware, Inc.] (http://www.kitware.com/)
- [Rensselaer Polytechnic Institute] (http://rpi.edu/)

### Licensing
*Coming soon*

## Resources
### Documentation
*Coming soon*

### Mailing Lists
The **iMSTK Users** mailing list is the principal means of communication among developers and users: <imstk-users@imstk.org>
- subscribe [HERE](http://public.kitware.com/mailman/listinfo/imstk-users)
- find mailman archive [HERE](http://public.kitware.com/pipermail/imstk-users/)

The **iMSTK Developers**  mailing list is for developers where design and implementation issues are discussed: <imstk-developers@imstk.org>
- subscribe [HERE](http://public.kitware.com/mailman/listinfo/imstk-developers)
- find mailman archive [HERE](http://public.kitware.com/pipermail/imstk-developers/)

### Issue-tracker
Designed more specifically for developers, the issue tracker allows developers to list and discuss issues & enhancements:
>https://gitlab.kitware.com/iMSTK/iMSTK/issues

*Assign labels to the issues. The description of each label can be found [HERE](https://gitlab.kitware.com/iMSTK/iMSTK/labels).*

<a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=bug"><span class="label color-label " style="background-color: #cc0033; color: #FFFFFF" title="Report an error at runtime" >bug</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=compilation"><span class="label color-label " style="background-color: #cc0033; color: #FFFFFF" title="Report an error during compilation" >compilation</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=critical"><span class="label color-label " style="background-color: #ff0000; color: #FFFFFF" title="Issue that should require the developers main focus" >critical</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=enhancement"><span class="label color-label " style="background-color: #5cb85c; color: #FFFFFF" title="Suggest an enhancement you believe is needed (new features...)" >enhancement</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=optimization"><span class="label color-label " style="background-color: #5cb85c; color: #FFFFFF" title="Report a slow process and possibly offer ideas to optimize it" >optimization</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=clean+up"><span class="label color-label " style="background-color: #428bca; color: #FFFFFF" title="Suggestions to improve the code style" >clean up</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=refactor"><span class="label color-label " style="background-color: #428bca; color: #FFFFFF" title="Suggest a better way to implement a certain feature" >refactor</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=testcase"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a test or example within the project" >testcase</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=visualization"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a visualization feature" >visualization</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=mechanics"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a mechanics feature" >mechanics</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=device"><span class="label color-label " style="background-color: #ffecdb; color: #333333" title="Suggestion/issue related to a device feature" >device</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=documentation"><span class="label color-label " style="background-color: #f0ad4e; color: #FFFFFF" title="Report an issue/requirement that is related to documentation (code, project...)" >documentation</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=support"><span class="label color-label " style="background-color: #f0ad4e; color: #FFFFFF" title="Report an issue/requirement that is related to support (dashboard, mailing list, website...)" >support</span></a> <a href="https://gitlab.kitware.com/iMSTK/iMSTK/issues?label_name%5B%5D=discussion"><span class="label color-label " style="background-color: #8e44ad; color: #FFFFFF" title="Start a discussion about a certain topic that requires other users and developers input" >discussion</span></a>

## Prerequisites
* Git
* CMake 3.2 or higher

##### On Linux:

```bash
sudo apt-get install build-essentials libgl1-mesa-dev libxt-dev libusb-1.0-0-dev
```

## Getting started with iMSTK
### 1. Getting the source code
To be able to contribute back to the iMSTK project, the preferred way is to use [Git] for code version control. You can use the following command in the terminal for Linux/MacOSx, or in [Git Bash] for Windows.
```sh
git clone git@gitlab.kitware.com:iMSTK/iMSTK.git
```

### 2. Setting up your SSH key
The build process will check out external dependency sources with the SSH protocol to avoid manually entering credentials during the build process. To allow this, make sure you set up your ssh key in your profile [HERE](https://gitlab.kitware.com/profile/keys). You can find documentation on how to generate and retrieve your public ssh key [HERE](https://gitlab.kitware.com/help/ssh/README).

### 3. Building iMSTK
We use [CMake] to configure the project on every platform. See how to run it [HERE](https://cmake.org/runningcmake/).
* ##### On Linux/MacOSx
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

* ##### On Windows
Run CMake-GUI and follow the directions described [HERE](https://cmake.org/runningcmake/). You will have to choose which version of Visual Studio you'd like to use when configuring the project, make sure to select **Microsoft Visual Studio C++ 12 2013**. CMake will generate a `iMSTK.sln` solution file for Visual Studio at the top level. Open this file and build all targets, which will checkout, build and link all iMSTK dependencies. When making changes to iMSTK [base source code](/Base), you can then build from the `iMSTK.sln` solution file located in the `Innerbuild` directory.
> /!\ MVSC 2015 is not yet supported as the dependency libusb 1.0.20 does not support it yet. We will work on supporting MVSC in the near future when libusb 1.0.21 is released.

* ##### Phantom Omni Support
To support the [Geomagic Touch (formerly Sensable Phantom Omni)](http://www.geomagic.com/en/products/phantom-omni/overview) haptic device, follow the steps below:
  1. Install the [OpenHaptics] SDK as well as the device drivers:
       - for [Windows](https://3dsystems.teamplatform.com/pages/102774?t=r4nk8zvqwa91)
       - for [Linux](https://3dsystems.teamplatform.com/pages/102863?t=fptvcy2zbkcc)
  2. Reboot your system.
  3. Configure your CMake project with the variable `iMSTK_USE_OMNI` set to `ON`.
  4. After configuration, the CMake variable `OPENHAPTICS_ROOT_DIR` should be set to the OpenHaptics path on your system.

* ##### Vulkan Renderer
To use the Vulkan renderer, follow these steps:
  * Download the [VulkanSDK](https://vulkan.lunarg.com/).
  * Download your GPU vendor's latest drivers.
  * Enable the `iMSTK_USE_Vulkan` option in CMake.

## Using iMSTK in your application
*Coming soon*

---
[NIH-OD]: <https://www.nih.gov/about-nih/what-we-do/nih-almanac/office-director-nih>
[NIH-NIBIB]: <https://www.nibib.nih.gov/>
[Rensselaer Polytechnic Institute]: <www.rpi.edu>
[Kitware, Inc.]: <www.kitware.com>
[Git Bash]: <https://git-for-windows.github.io/>
[Git]: <https://git-scm.com>
[CMake]: <https://cmake.org>
[Ninja]: <https://ninja-build.org/>
[OpenHaptics]: <http://www.geomagic.com/en/products/open-haptics/overview/>
