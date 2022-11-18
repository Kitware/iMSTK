# Continous Integration

## Table of Contents 

* [iMSTK Gitlab Runner Maintenance](#imstk-gitlab-runner-maintenance)  
    * [Overview](#overview)
    * [Adding a new configuration](#adding-a-new-configuration)
    * [Modify CMake variables of an existing configuration](#modify-cmake-variables-of-an-existing-configuration)
    * [Add a 3rd party dependency](#add-a-3rd-party-dependency)
    * [Gitlab Pages](#gitlab-pages)
* [Debugging](#debugging)
* [Reproducing an Environment](#reproducing-an-environment)
* [Render Tests](#render-tests)

## Introduction

iMSTK uses Gitlab's Continuous Integration (CI) pipelines to actively
verify project integrity. When changes are introduced to iMSTK we
automatically re-run CI pipelines to check that the project builds
and tests pass on our supported platforms.

Kitware CI documentation, debugging tips, and helpful hints can be found at https://gitlab.kitware.com/utils/gitlab-ci-docs/-/wikis/local-ci .

Pipeline results are found at https://gitlab.kitware.com/iMSTK/iMSTK/-/pipelines . Raw logs can be downloaded
from individual jobs in Gitlab.

iMSTK CI resources are available at https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/.gitlab .

## iMSTK Gitlab Runner Maintenance

The following sections describe gitlab CI and runner functionality in iMSTK and how to perform various tasks.

### Overview

The gitlab CI and runners are run using the root directories `.gitlab-ci.yml`. This file defines how and when various `stages` are run. Within our `gitlab-ci.yml` file there are various definitions with `extends` in them. These refer to definitions in other files for reusability. All of our gitlab-ci files are in the `iMSTK/.gitlab/` directory.

### Adding a new configuration

To add a new configuration that has test and build stages, define something such as:
```
windows-vs2022-ninja-unity:build:
    extends:
        - .windows_vs2022_ninja_unity
        - .cmake_build_windows
        - .cmake_build_artifacts
        - .windows_builder_tags
        - .run_manually

windows-vs2022-ninja-unity:test:
    extends:
        - .windows-vs2022-ninja-unity
        - .cmake_test_windows
        - .cmake_test_artifacts
        - .windows_builder_tags
        - .run_dependent
    dependencies:
        - windows-vs2022-ninja-unity:build
    needs:
        - windows-vs2022-ninja-unity:build
```
This defines a test and build stage named `windows-vs2022-ninja-unity` which extends `.windows-vs2022-ninja-unity` that has yet to be defined.

As this is a windows build it may be defined in `iMSTK/.gitlab/os-windows.yml`. Giving its own cmake configuration file.
```
.windows_vs2022_ninja_unity:
    extends: .windows_vs2022

    variables:
        CMAKE_CONFIGURATION: windows_vs2022_ninja_unity
```

To then provide it with a configuration file add the corresponding `.cmake` file in the `iMSTK/.gitlab/ci` directory. One may also include another cmake file to inherit/extend its properties.
```
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_vs2022_ninja.cmake")

set(iMSTK_BUILD_FOR_UNITY ON CACHE BOOL "")
```

### Modify CMake variables of an existing configuration

To modify an existing configuration one may alter the ".cmake" files located in "iMSTK/.gitlab/ci" directory.

### Add a 3rd party dependency

Sometimes the build requires things the user should install often due to things like licensing issues, or it not being practical to ship all the source code for everything together. Within the gitlab runner scripts you will fine that a `stage` defines a `script` that is run on the gitlab runner machine. For example doxygen should be easy to understand:
```
- apk update
- apk add doxygen ttf-freefont graphviz
- mkdir Docs/html
- doxygen Doxyfile
- mv Docs/html/ public/
```

If the 3rd party dependency is portable enough such as `SWIG`, `CMake`, `Ninja`, `Python`, or various python packages it is recommended you fetch and install them at the start of the script. This makes the build more portable and can run on larger amounts of systems. See `os-windows.yml` which refers to various scripts located in `iMSTK/.gitlab/ci/`. An example would be `cmake.ps1`, `cmake/sh`, `ninja.ps1`, `ninja.sh`, or `download_python.cmake`. If cmake is installed first I would recommend using it. It is highly recommended to check hashes if pulling from an external location.

If the 3rd party dependency is something like a compiler, driver, or not very portable. It is recommend to install it on the gitlab runner system and when setting up/registering the gitlab runner. Specify tags for it. Existing runners & tags in use can be viewed by navigating to `Settings->CI-CD->Runners` in the repository, try to stay consistent. For example, `cmake`, `linux`, `docker`, `msvc-19.31`, `csharp-17`, `vs2022` are all tags that may be used to inform whoever is about to run something what the machine has. Within our gitlab runner scripts there is then a location to specify tags, such that it will search for a machine that satisfies the tag for a given jobs. Tags for windows can be found in `os-windows.yml`.

### Gitlab Pages

`gitlab.kitware.com` does not support gitlab pages. For this reason the repository is mirror'd to public "gitlab.com" (also possible with github). Our gitlab runner code then has a rule to only allow the `pages` stage to run on that mirror.

This job installs doxygen (code documentation) and mkdoc (user documentation). Then builds the html pages. Finally copies them to the `public/` directory to be hosted.

It also runs a couple scripts to generate `.md` documentation from examples. It does this by searching the `./Examples` directory of imstk for `desc.md` files. One can add "[cpp_insert]: <FileNameHere.cpp>" to dump the contents of a file local to the `desc.md`'s directory into a code block in the `desc.md` file. It also generates an index/nav for it in the mkdocs.yml.

It is recommended to keep media content file size small as it currently exists alongside the iMSTK repository. The documentation could be moved to git LFS or even a separate repository with optional pull of it (most will probably read the online hosted documentation anyways).

The mirror can be alter/maintained in `Settings->Repository->Mirroring Repositories` by those with Maintainer rights on the repository. If the mirror needs to be reconfigured, one can setup a push mirror using their git credentials for `gitlab.com` (not gitlab.kitware.com). This user must be a maintainer in the mirror repository as well.

Setup a push mirror with (both user and pass credentials will be hidden):
 * url: "https://USERNAME@gitlab.com/imstk/imstk.gitlab.io.git"
 * password: \<your password\>

## Debugging

Common CI failures are discussed at https://gitlab.kitware.com/utils/gitlab-ci-docs/-/wikis/debugging .

### Reproducing an environment

In many cases it is useful to reproduce a platform environment in order to recreate build or test failures
on your local machine. We can use a container engine such as Docker or Podman to build and test iMSTK
locally using the same environment and tools as the iMSTK Linux CI pipeline.

The following command configures the Fedora Linux container and starts a shell session in the container:
```bash
podman run --rm -it -v /path/to/iMSTK:/iMSTK -v /path/to/iMSTK-build:/iMSTK-build -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY kitware/imstk:ci-fedora36-20220621
```

We can break down each part of the command as follows:
- `podman`: Call the container engine. See installation instructions at https://podman.io/getting-started/installation
- `run`: Tell the engine that we want to run a command in a new container. https://docs.podman.io/en/latest/markdown/podman-run.1.html
- `--rm`: Clean up and remove the container on exit so that a new, clean container is created each time the command is run. can be omitted to re-use a locally configured container over multiple runs.
- `-it`: Keeps `stdin` open and allocates a pseudo-TTY for `stdout`. Allows `stdin` and `stdout` to be used in our interactive shell.
- `-v /path/to/iMSTK:/iMSTK`: Bind mount the local path to the iMSTK source repository on the host machine to the path `/iMSTK` in the container. Once the shell launches you can use `ls /` to verify that the iMSTK source directory is at the container root.
- `-v /path/to/iMSTK-build:/iMSTK-build`: Bind mount the local path to the iMSTK build repository on the host machine to the path `/iMSTK-build` in the container. For a new build your `iMSTK-build` directory should be empty as initialization.
- `-v /tmp/.X11-unix:/tmp/.X11-unix`: Bind mount the X11 directory so that the container can access display sockets from the host machine. Note that Unix paths are case-sensitive. Necessary for running iMSTK tests and examples with graphics. See https://gitlab.kitware.com/utils/gitlab-ci-docs/-/wikis/local-ci#forwarding-machine-resources for more information.
- `-e DISPLAY`: Set the DISPLAY environment variable to the value on the host machine. The value of DISPLAY indicates the X11 socket to use for window graphics such as `:X0.0` on physical access to the host or `:X10.0` on remote access. See https://gitlab.kitware.com/utils/gitlab-ci-docs/-/wikis/local-ci#forwarding-machine-resources for more information.
- `kitware/imstk:ci-fedora36-20220621`: The image to use for creating the container. At the time of writing iMSTK uses a Fedora Linux 36 image for Linux CI testing. You can find the most up-to-date image name at `iMSTK/.gitlab/os-linux.yml`.

By omitting a command at the end of the docker image we enter an interactive shell for building and testing. Set up build tools with the following commands:
```bash
> pushd /iMSTK
> .gitlab/ci/cmake.sh   # Downloads and installs compatible CMake distribution
> .gitlab/ci/ninja.sh   # Downloads and installs compatible Ninja distribution for building
> PATH=$PATH:/iMSTK/.gitlab/:/iMSTK/.gitlab/cmake/bin   # Sets up container PATH variable to reference CMake and Ninja binaries
> popd
```

Lastly, we move into the build folder and build.
```bash
> cd /iMSTK-build
> cmake /iMSTK
> cmake --build . --config "Release"
```

### Render Tests

It may be necessary to temporarily disable xhost access control to allow the container to run render tests
from the host's display. Access control can be updated from the host while the container is running.

To disable access control run this command from a separate host shell prompt:
```bash
xhost +
```

To enable again after render tests in the container:
```bash
xhost -
```