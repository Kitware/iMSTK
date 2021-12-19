# iMSTK Maintenance

The following sections describe various iMSTK maintenance tasks

# Adding a new iMSTK Dependency

For clarity the new library will be called `NewLib`. Depending on the use case some of the following steps may be skipped, but that is unlikely.

### Superbuild

iMSTK's CMake-based build process proceeds in two stages. The first stage downloads, builds and installs all of iMSTK's dependencies. This stage is called 'Superbuild'. Superbuild allows developers to build iMSTK and its dependencies in one go. When a new dependency needs to be added, the dependency list needs to be updated so that the superbuild builds and installs it for iMSTK to use. This is described in sections below.

### Innerbuild

iMSTK's code gets build in the second stage of the build process called 'Innerbuild'. The innerbuild follows the superbuild and expects all dependencies to be built, installed, and findable.

## Updating the CMakeLists.txt

In the `CMakeLists.txt` in the root directory. While we are using `${PROJECT_NAME}` to reflect the `iMSTK` project in this file, everywhere else `iMSTK` will be more appropriate.


**Step 1:** Define the external library as a iMSTK dependency

---
    if (${PROJECT_NAME}_USE_NewLib)
      imstk_define_dependency(NewLib)
    endif()

This tells the superbuild that `iMSTK` depends on `NewLib` and will trigger all the other processes namely building, installing, finding and linking to it.


**Step 2:** Add `find_package` with the appropriate options for this library

--- 

    if (${PROJECT_NAME}_USE_VRPN)
      find_package( NewLib REQUIRED )
    endif()

This is executed in the innerbuild of `iMSTK`. It will enable the innerbuild to find the components of the library. Please note that `find_package` should be executed in the local `FindNewLib.cmake` that will be created in a later step, and not the CMake top-level script or the find scripts of the library that is being built.

**Step 3:** Add an option to turn the building of dependency ON/OFF (Optional)

---
Add a Setting `${PROJECT_NAME}_USE_NewLib` as
	
    option(${PROJECT_NAME}_USE_NewLib "Build with NewLib support" OFF)
    mark_as_superbuild(${PROJECT_NAME}_USE_NewLib)
	
The state of the option (`ON`/`OFF`) should reflect whether the new library should be built by default or not. If a dependency is a required one, one may omit this. If the dependency is optional, one can conditionally execute other steps in the CMake build process by surrounding the statements with `if(${PROJECT_NAME}_USE_NewLib)` or `if(iMSTK_USE_NewLIb)`.

Variables that need to be passed from the superbuild to the innerbuild need to be "marked" as such using `mark_as_superbuild`.

**Step 4:** Edit CMake\External\CMakeLists.txt

---

To enable CMake to find the library correctly _if_ the library already provides a `Config.cmake` or a `Find` pass the path to the library into the innerbuild, so use 

    -DNewLib_DIR:PATH=${NewLib_DIR}  

**Step 5:** Add `CMake/External/External_NewLib.cmake`

You will need to create this file which describes what files to download from where and how to build them to support your new library in the superbuild. In general this will mean customizing `imstk_add_external_project`.

    include(imstkAddExternalProject)

    # Download options
    if(NOT DEFINED iMSTK_NewLib_GIT_SHA)
      set(iMSTK_NewLib_GIT_SHA "...")
    endif()
    if(NOT DEFINED iMSTK_NewLib_GIT_REPOSITORY)
      set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
        URL https://gitlab.kitware.com/iMSTK/newlib/-/archive/${iMSTK_NewLib_GIT_SHA}/newlib-${iMSTK_NewLib_GIT_SHA}.zip
        URL_HASH MD5=...
        )
    else()
      set(EXTERNAL_PROJECT_DOWNLOAD_OPTIONS
        GIT_REPOSITORY ${iMSTK_NewLib_GIT_REPOSITORY}
        GIT_TAG ${iMSTK_NewLib_GIT_SHA}
        )
    endif()

    imstk_add_external_project(NewLib
      ${EXTERNAL_PROJECT_DOWNLOAD_OPTIONS}
      CMAKE_CACHE_ARGS
        ...
      DEPENDENCIES ${NewLib_DEPENDENCIES}
      ...
    )

and customizing the build options for the new library by passing them via the `CMAKE_CACHE_ARGS` section. e.g.

        -DBUILD_TESTING:BOOL=OFF

There are quite a few examples for this in `iMSTK` now, best is to start simple and extend depending on the needs of the new library.  `imstk_add_external_project` can be found in `CMake/Utilities/imstkAddExternalProject.cmake`

**Step 6:** `CMake/FindNewLib.cmake`

In case `NewLib` _does not_ provide a cmake `NewLibConfig.cmake` or its own find you will need to create `FindNewLib.cmake` in the `CMake` directory, this will be used during the innerbuild configuration step to initialize the include directories and library files related to the new library. The general pattern is like this. `imstkFind` can be found in `CMake/Utilities`


    include(imstkFind)

    # Locate the header newlib.h inside the newlib subdirectory
    imstk_find_header(NewLib newlib.h newlib)
	# Find liba and add it to the libraries for NewLib
    imstk_find_libary(NewLib liba)
	# Finish the process
    imstk_find_package(NewLib)

**Step 7:** Edit `CMake/iMSTKConfig.cmake.in`

To expose the state of any variables that you set in the superbuild to project that depend on `iMSTK` you need to store the state inside this file. At installation time, this will be written out and is used to correctly restore the state at build time. e.g. 

    set(iMSTK_USE_NewLib @iMSTK_USE_NewLib@)

Any `find_package` commands issued in the main file have to be replicated here are well so that users of `iMSTK` can access all of its dependencies.

# Updating a Dependency in iMSTK

## Updating a dependency in imstk

 - Swap the url in ` /CMake/External/<DependencyLibraryName>.cmake` to a different url.
    -  Ex: `https://gitlab.kitware.com/iMSTK/assimp/-/archive/fixCompilationError/assimp-fixCompilationError.zip`. Pulling zips prevents git history with it which can be sizeable.
    - Alternatively git shallow would be used to avoid large git histories. But it fails when the tree gets too large, thus if the HEAD of the branch pulled from moves too far, it will fail to pull. (used to cause old version of iMSTK to fail to build until we switched to zips)
 - Update the md5 hashsum in that same file (if using zips).
    - The hashsum is a nearly unique identifer generated from the files of the dependency. This mostly is a security measure that ensures you are getting the files you expect to be getting.
    - To acquire the md5 hashsum one can build iMSTK with the newly updated url. It will fail on the md5 check and report both the actual and current md5.

## Updating a Remote Fork

Most dependencies in iMSTK are forked. This way we don't depend on the remote repository as it could change (rebased/amended/moved/deleted). A few forks also contain our own diffs in the rare case something like a CMake fix is introduced. To update a fork:
 - Clone the fork locally: `git clone <git url of fork>` (all forks found in iMSTK group here: https://gitlab.kitware.com/iMSTK)
 - Add upstream `git remote add upstream <url of actual repo/repo forked off>` (the description of the fork normally provides what it was forked from)
    - Tip: Issue `git remote -v` to list all remotes
 - Merge upstream (or rebase): `git merge upstream/<branch to update from>`
 - Push your changes

After updating your fork you can proceed with the beginning of this guide on how to pull a different source.

# Adding data to the data repository

iMSTKs data sits in a separate repository https://gitlab.kitware.com/iMSTK/imstk-data. It is downloaded by iMSTK's superbuild when either the testing or the examples is enabled. To add new data to the repository two steps are necessary

**1. Add Data to repository**

The repository is already checked out as an external dependency in your build directory (if using superbuild) as `<build_dir>\External\iMSTKData\src\Data`, data can be added to the folder here and directly commited and pushed. 

**2. Update SHA in `ExternalData.cmake`**

The file that controls the downloading of the data is `CMake/External/External_iMSTKData.cmake`. After commiting and pushing a change in the `iMSTKData` repository, the SHA to checkout needs to be updated to the SHA matching the latest commit.
