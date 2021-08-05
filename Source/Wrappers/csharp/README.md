# How to build iMSTK C# wrappers?
- Windows:
    - download and install [swig](http://www.swig.org/download.html)
    - Edit the system environment variables:
        - create 2 "System Variables": `SWIG_DIR` points to `/path/to/swig/dir/`, and `SWIG_EXECUTABLE` points to `/path/to/swig/dir/swig.exe`
        - add one entry to PATH variable, `/path/to/swig/dir`
        - may have to restart the computer to apply all environment variables.
    - Build imstk with `iMSTK_WRAP_CSHARP=ON`
- Ubuntu
    - sudo apt install swig
    - Build imstk with `iMSTK_WRAP_CSHARP=ON` (cmake can find swig without any issue on my system).

# How ro run C# examples on Windows 
- Install mono. See [https://www.mono-project.com/docs/getting-started/install/windows/](https://www.mono-project.com/docs/getting-started/install/windows/)
- The C# examples run without building iMSTK csharp wrappers, but needs iMSTK built with OpenHaptics.
    - clone the repository [git@gitlab.kitware.com](mailto:git@gitlab.kitware.com):jianfeng.yan/imstk_csharp.git
    - build this specific version of iMSTK: [https://gitlab.kitware.com/jianfeng.yan/iMSTK/-/tree/csharp_wrapper](https://gitlab.kitware.com/jianfeng.yan/iMSTK/-/tree/csharp_wrapper)
    - build imstk_csharp/imstkCWrapper. `iMSTK_DIR` has to set to /imstk_build/install/lib/cmake/iMSTK-4.0.
    - There are several examples in imstk_csharp/cs_examples. Taking pbdCloth.cs for an example, open a terminal, go to imstk_csharp/cs_examples
        - /path/to/mono/bin/csc cs_examples\pbdCloth.cs cs_files\*.cs -out:bin\pbdCloth.exe
        - bin\pbdCloth.exe can run directly.
- You can also build iMSTK C# wrappers yourself (without OpenHaptics), and then
    - copy all C# files in Innerbuild/Source/CSWrapper to imstk_csharp/cs_files.
    - copy `imstkCWrapperCSHARP_wrap.cxx` and `imstkCWrapperCSHARP_wrap.h` to imstk_csharp/imstkCWrapper

    and then build imstkCWrapper and run examples as described above.

# How to compile SWIG generated C# code into a dll, and run examples with it?
- open a terminal, cd to imstk_csharp
- /path/to/mono/bin/csc -target:library cs_files\*.cs -out:bin\iMSTKCS.dll (This compiles the C# code into a dynamic lib, iMSTKCS.dll)
- /path/to/mono/bin/csc -r:bin\iMSTKCS.dll pbdCloth.cs -out:bin\pbdCloth.exe
- bin\pbdCloth.exe can run directly.
