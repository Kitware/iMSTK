# Adding a new Dependency to iMSTK

In this document the new library will be called `NewLib`. Depending on the use case some of the following steps may be skipped, but that is unlikely.

### Superbuild

Its the superbuilds job to download all the noted dependency, build and install them. Once that is done it will configure and build the Innerbuild.

While a lot of projects at kitware use a superbuild architecture there are slight differences between various implementations, very likely most or all of the following steps are applicable to other setups but they may or may not happen in the same plce as iMSTK

### Innerbuild

The innerbuild is the actual `iMSTK` code, it expects all dependencies to be built, installed, and findable.

## CMakeLists.txt

In the `CMakeLists.txt` in the root directory. While we are using `${PROJECT_NAME}` to reflect the `iMSTK` project in this file, everywhere else `iMSTK` will be more appropriate.

---

Add a Setting `${PROJECT_NAME}_USE_NewLib` e.g.
	
    option(${PROJECT_NAME}_USE_NewLib "Build with NewLib support" OFF)
	
The state of the option (`ON`/`OFF`) should reflect whether the new library is should be built by default. When adding a new required dependency the option may be omitted, in that case omit the `if()` check for all the other steps

This enables us to control and check for the availability of the library, any section that is only to be performed when `NewLib` is being built should be checked via `if(${PROJECT_NAME}_USE_NewLib)` or `if(iMSTK_USE_NewLIb)`

---

Define `NewLib` as a dependency 

    if (${PROJECT_NAME}_USE_NewLib)
      imstk_define_dependency(NewLib)
    endif()

This tells the superbuild architecture that `iMSTK` depends on `NewLib` and will trigger all the other processes.

---

Add `find_package` with the appropriate options for this library 

    if (${PROJECT_NAME}_USE_VRPN)
      find_package( NewLib REQUIRED )
    endif()

This is actually executed in the innerbuild of `iMSTK` it will enable the innerbuild to find the components of the library. Please note that `find_package` should executed is the local `FindNewLib.cmake` that will be created in a later step, and not the Cmake global script or the find scripts of the library that is being built.

## CMake\External\CMakeLists.txt

This file adds the project that makes up the innerbuild to the superbuild, any variables that need to be passed from the superbuild to the innerbuild need to be set here. This means you will probably want to add 

    -D${PROJECT_NAME}_USE_NewLib:BOOL=${${PROJECT_NAME}_USE_NewLib} 

to the section in `ExternalProject_Add`, so that the appropriate option is visible inside the innerbuild.

Additionally to enable CMake to find the library correctly _if_ the library already provides a `Config.cmake` or a `Find` pass the path to the library into the innerbuild, so use 

    -DNewLib_DIR:PATH=${NewLib_DIR}  

## `CMake/External/External_NewLib.cmake`

You will need to create this file, this is what describes what files to download and how to build them to support your new library to the superbuild. In general this will mean customizing `imstk_add_external_project`.

    include(imstkAddExternalProject)
	imstk_add_external_project(NewLib
	  ....
	)

and customizing the build options for the new library by passing them via the `CMAKE_CACHE_ARGS` section. e.g.

        -DBUILD_TESTING:BOOL=OFF

There are quite a few examples for this in `iMSTK` now, best is to start simple and extend depending on the needs of the new library.  `imstk_add_external_project` can be found in `CMake/Utilities/imstkAddExternalProject.cmake`

## `CMake/FindNewLib.cmake`

In case `NewLib` _does not_ provide a cmake `NewLibConfig.cmake` or its own find you will need to create `FindNewLib.cmake` in the `CMake` directory, this will be used during the innerbuild configuration step to initialize the include directories and library files related to the new library. The general pattern is like this. `imstkFind` can be found in `CMake/Utilities`


    include(imstkFind)

    # Locate the header newlib.h inside the newlib subdirectory
    imstk_find_header(NewLib newlib.h newlib)
	# Find liba and add it to the libraries for NewLib
    imstk_find_libary(NewLib liba)
	# Finish the process
    imstk_find_package(NewLib)

## `CMake/iMSTKConfig.cmake.in`

To expose the state of any variables that you set in the superbuild to project that depend on `iMSTK` you need to store the state inside this file. At installation time, this will be written out and is used to correctly restore the state at build time. e.g. 

    set(iMSTK_USE_NewLib @iMSTK_USE_NewLib@)

any `find_package` commands issued in the main file have to be replicated here are well, so that users of `iMSTK` can access all of `iMSTK`s dependencies.
