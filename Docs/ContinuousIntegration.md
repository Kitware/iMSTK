# iMSTK Continous Integration

iMSTK uses Gitlab's Continuous Integration (CI) pipelines to actively
verify project integrity. When changes are introduced to iMSTK we
automatically re-run CI pipelines to check that the project builds
and tests pass on our supported platforms.

Kitware CI documentation, debugging tips, and helpful hints can be found at https://gitlab.kitware.com/utils/gitlab-ci-docs/-/wikis/local-ci .

Pipeline results are found at https://gitlab.kitware.com/iMSTK/iMSTK/-/pipelines . Raw logs can be downloaded
from individual jobs in Gitlab.

iMSTK CI resources are available at https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/.gitlab .

--- 

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
- `kitware/imstk:ci-fedora36-20220621`: The image to use for creating the container. iMSTK uses a Fedora Linux 36 image for Linux CI workflows.

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

