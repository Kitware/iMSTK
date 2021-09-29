# General 

Please note that the C# support is still a work in progress, while we will do our best to maintain the C/C++ layer of this part of iMSTK it might happen that some of the code in the wrapper comes out of sync with the rest of the system, we appreciate any reports or fixes that you want to contribute to help us out. The current version of the wrapper will be fully tested against the `5.0` release tag of imstk.

## Components

iMSTK uses the [simplified wrapper and interface generator (SWIG)](http://www.swig.org/) to provide an interface between C++ and C#. This enables us to use most of the C++ interface almost "as is" in C#. There are a few components to this wrapping proccess. The sources to generate the wrapper can be found under `Source/Wrappers/csharp` in the iMSTK source directory. Even on windows not all of the processes are automated yet.

The directory contains the following items 
    - SwigInterface: The swig `.i` files that tell swig how to build the interfaces for iMSTK, this is the C/C++ part of the wrapper
    - iMSTKCSharp: A CMake project (Visual Studio only) that can build a combined dll with all the swig generated files, the C# part of the wrapper
    - Examples: A set of iMSTK examples written in C#
    - Testing: Unit tests for some critical parts of the wrapper (can't be built automatically atm)

## Building

Support for building differs between Linux and Visual Studio, as Visual Studio supports C# projects. Both require SWIG to be installed. On windows two different paths are available, one using visual studio and `.NET` libraries, or a more manual approach by building via `Mono`. In both cases the C/C++ layer has to be built first.

# How to build iMSTK C/C++ wrappers?
- Windows:
    - download and install [swig](http://www.swig.org/download.html)
    - Edit the system environment variables:
        - create 2 "System Variables": `SWIG_DIR` points to `/path/to/swig/dir/`, and `SWIG_EXECUTABLE` points to `/path/to/swig/dir/swig.exe`
        - add one entry to PATH variable, `/path/to/swig/dir`
        - may have to restart the computer to apply all environment variables.
    - Build imstk with `iMSTK_WRAP_CSHARP=ON`
- Ubuntu
    - sudo apt install swig
    - Build imstk with `iMSTK_WRAP_CSHARP=ON`.

The output of this step is a shared library `iMSTKCWrapper.[dll|sh]` (located in `/bin`), and a series of `.cs` files that together with the `iMSTKCWrapper` library now provide access to iMSTK C++ functionalies. Note that iMSTK depends on a variety of other libraries, these are not statically linked in the `iMSTKCWrapper` library, whenever writing C# code, the `iMSTKCWrapper` library and all pertinent iMSTK dependencies need to be accessible (e.g. by being on the library path).

Some people like to compile all the `.cs` files into a separate library, the Visual Studio build does that in a separate target.

# How to run C# examples on Windows

## Via Visual Studio .NET
- The superbuild needs to be run twice for the C# Examples to be run from the install folder
- If you are looking at the inner build, after building `iMSTKCWrapper`, you will need to run CMake "Configure" and "Generate" again to load the example projects.

## Via Mono 
- Install mono. See [https://www.mono-project.com/docs/getting-started/install/windows/](https://www.mono-project.com/docs/getting-started/install/windows/)
- The C# examples run without building iMSTK csharp wrappers, but needs iMSTK built with OpenHaptics.
    - build iMSTK with `iMSTK_WRAP_CSHARP=ON`
    - There are several examples in `Source/Wrappers/csharp/Examples`. Taking pbdCloth.cs for an example, open a terminal, go to the imstk installation directory and execute 
        - /path/to/mono/bin/csc '<imstkSource>\Source\Wrappers\csharp\Examples\pbdCloth.cs include\iMSTKSharp\*.cs -out:bin\pbdCloth.exe
        - bin\pbdCloth.exe can run directly.
		
# How to compile SWIG generated C# code into a dll, and run examples with it?
- open a terminal, cd to the iMSTK installation directory
- /path/to/mono/bin/csc -target:library include\iMSTKSharp\*.cs\*.cs -out:bin\iMSTKCS.dll (This compiles the C# code into a dynamic lib, iMSTKCS.dll)
- /path/to/mono/bin/csc -r:bin\iMSTKCS.dll <imstkSource>\Source\Wrappers\csharp\Examples\pbdCloth.cs -out:bin\pbdCloth.exe
- bin\pbdCloth.exe can run directly.
