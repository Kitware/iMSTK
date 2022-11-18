# About the Examples

The iMSTK source includes a number of [examples](https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/Examples) located within the repository itself. Many of these examples won't show up in visual studio's when not building with certain configurations. These examples along with the tests guide the toolkit and can be a good source of learning. The examples also help answer questions, as people can be referred to specific ones, it's also encouraged to provide images, gifs, or video references demonstrating example functionality.

There are additionally some [C# examples](https://gitlab.kitware.com/iMSTK/iMSTK/-/tree/master/Source/Wrappers/iMSTKCSharpWrapper/Examples) within our wrapper code.

## Example Documentation

Optionally an example directory may have a desc.md file. The gitlab runner that builds the documentation will generate documentation pages for every example with one. It will also place it in the examples section. Any `[cpp_insert]: <myfileHere.cpp>` will get placed as code in the final user documentation. Images are relative to docs/Examples/ directory that is created.