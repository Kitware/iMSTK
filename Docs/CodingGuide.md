# Introduction

This guide collates best practices from a variety of sources, while some of these express a certain _taste_ we want the items that are covered here to be consistent accross iMSTK. 

Coding style is not only limited to the actual formatting but also to things like the naming of functions or variables, or the use of certain c++ features or patterns. By being consistent in these we make development for iMSTK easier for ourselves and others as names and patterns are reused accross the code base. 

While the text of this guide is mostly culled from the [OpenSurgSim coding guide](https://app.assembla.com/spaces/OpenSurgSim/wiki/Coding_Standards) the following resources were used to create and refine the style 

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Google C++ Style guide](https://google.github.io/styleguide/cppguide.html)
- [High Integrity C++ Coding Standard](https://www.perforce.com/resources/qac/high-integrity-cpp-coding-standard)

## How to apply these standards

*  When working with existing code, fix its shortcomings as necessary.
*  When extending code created externally to the project (e.g. VTK, Eigen), try to remain consistent with the naming conventions of the external code.
*  Be flexible and reasonable. If the best thing for code readability is to violate these guidelines, please do so but that is rarely the case.
*  When in doubt, ask someone else for a second opinion.

# Layout, white space and comments

First and foremost, be reasonable and pay attention to clarity. White space and comments are to aid in code readability, not detract from it.

## Layout

- Use uncrustify to fix the formatting before you submit a merge request, the `CodeFormatEnforcer` target will correct all formatting issues. 

## Comments

* In general, the code should be made as self-documenting as possible by appropriate name choices and an explicit logical structure. 
* All comments should be written in US English, be spelled correctly, and make grammatical sense.
* Use doxygen style comments.

    Example:
```
/// \brief Does foo and bar.
///
/// Does not do foo the usual way if \p baz is true.
/// Add reference to implemented papers 
///
/// Typical usage:
/// \code
///   fooBar(false, "quux", Res);
/// \endcode
///
/// \note Take care not to create a division by zero
///
/// \param quux kind of foo to do.
/// \param [out] result filled with bar sequence on foo success.
///
/// \throws if a division by zero occurs
///
/// \returns true on success.
/// 
/// \warning missing feature 
bool 
fooBar(bool baz, StringRef quux, std::vector<int>& result);
```

* Prefer using `//` for all embedded comments, including multi-line comments, using `/*`, `*/` prevents the use of that style comment around it.

    Example:
```
// Comment spanning
// more than one line.
```

* Document responsibilities and background rather than what the code does, escpecially inline. The code should be readible and its details shouldn't need documentation. 

    Example : 
```
///
/// \class Viewer
///
/// \brief Base class for viewer that manages render window and the renderers
/// Creates backend-specific renderers on a per-scene basis.
/// Contains user API to configure the rendering with various backends
///
```

* Extensive comment blocks should generally only be used to explain the "rationale" behind an algorithmic choice or to explain an algorithm.

* Doxygen supports inline [MathJax](https://www.doxygen.nl/manual/formulas.html) use it to write out formulas in critical algorithm implementations.

* `///<` Can be used to create a comment within a line

```
int m_complexVar; ///< The sum of all things
```
* `///@{` and `///@}` can be used to group entries, for example when functions are overridden

```
/// Sets the point of interest
///@{
void setPoint(const Eigen::Vector3d& p);
void setPoint(double x, double y, double z);
///@}
```

* See the [Doxygen Commands](https://www.doxygen.nl/manual/commands.html) or a [Quickreference](https://www.mitk.org/images/1/1c/BugSquashingSeminars%242013-07-17-DoxyReference.pdf)

# Naming conventions
## General naming conventions

Consistent naming enables users to guess names and once they are familiar, correctly named functions and variables also should enhance readibility of the code

* Names representing types should be in mixed case starting with upper case.

    Example: `Line`, `SurfaceMesh`

* Variable names should be in mixed case starting with lower case.

    Example: `line`, `surfaceMesh`, `m_surfaceMesh`

* Named constants (including enumeration values) must be all uppercase using underscore to separate words.

    Example: `MAX_ITERATIONS`, `COLOR_RED`, `PI`

    * Avoid named constants in public interfaces.

* Macro names follow the same rules as constants.
* All macro names should be prefixed with `IMSTK_` this prevents clashes with macros imported from other projects
* Names representing methods or functions should be verbs and written in mixed case starting with lower case.

    Example: `getName()`, `computeTotalWidth()`

* Names representing template types should usually be a single uppercase letter, use descriptive nouns when it adds to clarity

    `template <class T>`

    `template <int N, class C, typename D>`

* Abbreviations and acronyms should not be all uppercase when used as a part of a name.

    `exportHTMLSource();` (NOT: `exportHtmlSource();`)

    `openDVDPlayer();` (NOT: `openDVDPlayer();`)

* Data members in a class should have names starting with "m_".

    `class SomeClass { private: int m_length; }`

* Data members in a data-only struct should not start with "m_".

* All names should be written in US English and spelled correctly.

    `fileName` (NOT: `imeDatoteke`)

* Variables with a large scope should have long (and descriptive) names; iteration variables with a short scope can have short names (i, j, ...).
* The name of the object is implicit, and should be avoided in a method name.

    `segment.getLength();` (NOT: `segment.getSegmentLength();`)

    `curve.getSegmentLength();` (this is OK, method name doesn't refer to the object)

* Rule for namespace names is same as type names.

* Top level namespace for iMSTK code is `imstk`.

* Math drive code is an exception to the naming rules, to mainting correspondence with publish formulas, appropriate variable names may be used, be that single letter variables or capitalized, e.g. the implemented formula may use `F` and `f` and so should the code.

## Specific naming conventions

* Plural form should be used on names representing a collection of objects.

    `vector<Point> points;`

    `int values[];`

* The prefix "num" should be used for variables representing a number of objects.

    `numPoints` (NOT: `nLines` and NOT: `nbLines`)

* Iteration variables of any integer type should be called i, j, k etc.
    * Variables named j, k etc. should be used for nested loops only.
* Variables that represent iterators for short loops may be abbreviated to "it"

    `for (vector<Foo>::iterator it = list.begin(); it != list.end(); ++it) ...`

    * Use more expressive names when using multiple iterators in the same construct

* Accessors and modifiers must start with "get" and "set".

* Accessors that return boolean must start with an interoggative such as "is" or "are".

* Try to use complementary names for complementary operations.
    * get/set, add/remove, create/destroy, start/stop, insert/delete, increment/decrement, old/new, begin/end, first/last, up/down, min/max, next/previous, old/new, open/close, show/hide, suspend/resume, etc.

* Try to avoid abbreviations in names.
    * Do not abbreviate common words:

    Write `command`, not `cmd`, `copy`, not `cp`, `point`, not `pt`, `compute`, not `comp`

    * Do use acronyms that have become more common than their expansions, at least in the jargon of the problem domain:

    Write `HTML`, not `HypertextMarkupLanguage`, or `CPU`, not `CentralProcessingUnit`

* Do not encode the type of the variable in the name

    `Line* line;` (NOT: `Line* pLine;` and NOT: `Line* linePtr;`)

* Avoid negated boolean names.

    `bool isSuccess` or `bool isError` (NOT: `isNoError`)

    `bool isMissing` or `bool wasFound` (NOT: `wasNotFound`)

    It is not immediately apparent what `!isNotFound` means.

* Enumeration constants should be prefixed by the enumeration type name.

    `enum Color { NONE = 0, COLOR_RED, COLOR_GREEN, COLOR_BLUE, MAX_COLORS };`

    If the final enumeration value is used only to track the number of other values, it can be named `MAX_<type>S`; this is clearer than `<type>_MAX`.

# Declarations

## Free functions and member functions
* Use "()", not "(void)", for an empty argument list.

  `void foo();`  (NOT: `void foo(void);`)

* Methods overriding virtual methods from a base class should be declared as `override` only (`virtual` keyword omitted).

   Example:
```
class Y : public X
{
    void complain() const override;
    ...
}
```

* Declare arguments as `const` when appropriate.

* Declare member functions as `const` when appropriate.

# Statements

## Types
* Type conversions should be done explicitly.

    `floatValue = static_cast<float>(intValue);` (NOT: `floatValue = intValue;`)

* Do not use old style casts. NOT: `(float)intValue`.

* Be aware that numeric values can overflow/underflow when using `static_cast`. You may need to verify the new type can hold the result.

* Be aware that arithmetic can result in invalid or wrapped values. You may need to verify the result will be within a valid range before performing arithmetic.

* We prefer using integer values that match the processor's word size: `size_t` for unsigned values and `ptrdiff_t` for signed values.

* For quantities and indices, we prefer `imstk::index_t` which is an unsiged datatype.

* In Standard Library, `size_type` corresponds to `size_t`, and `difference_type` corresponds to `ptrdiff_t`.

* In `Eigen::Index` and `Eigen::DenseIndex` corresponds to `ptrdiff_t`.

* `auto` use when the type is clear, or can't be written out

## Variables

* Variables should not have dual meaning.

    * Do not reuse the same variable for a different role.

* Use of global variables should be minimized. The use of a global across modules is an indication of a design flaw.

* Avoid declaring class data members (class variables) public.

* Variables should be declared in the smallest scope possible.

## Loops
* In a for loop, the initialization, condition and update should contain only loop control statements.

    Example:
```
int sum = 0;
for (int i = 0; i < 100; i++)
{
    ...
// NOT: for (int i = 0, sum = 0; i < 100; i++)
```

* Avoid "do-while" loops.

* In order prefer 
    * algorithms 
    * range based for loops
    * index acces 

## Conditionals
* Avoid including side effects in conditionals.

    Example:
```    
File* fileHandle = open(fileName, "w");
if (!fileHandle)
{
    ...
// NOT: if (!(fileHandle = open(fileName, "w"))) ...
```

* when dealing with pointers the following is an acceptable use within if

```
if (auto ptr = getPtr())
{
    ptr->useTheFunction();
}
``` 
  in this case the body of the `if` statement will only be executed if the `ptr` variable converts to `true` (i.e. is `!= nullptr`) this replaces 
```
PtrType* ptr = getPtr();
if (ptr != nullptr)
{
    ptr->useTheFunction();
}
```

# Constructors

* Ideally all or most member variables should be intialized after the constructor has run

* Note that `Eigen` matrices do not initialize to `0` the default constructor intentionally leaves the content at a random value

* Prefer the default member initializer over constructor inizialization, this is safer when multiple constructors need to be implemented and reduces the footprint of the member initializer list.

Example:
```
class A
{
    int m_a = 5;
}
```
NOT
```
class A
{
    A() : m_a(5) {}
    
    int m_a;
}
```

## Miscellaneous

* Avoid the use of magic numbers in the code. Consider declaring numbers other than 0 and 1 as named constants instead.

* Avoid any obviously bad programming practices (goto, default-int, etc).

* In C++ code, use `nullptr` instead of `NULL` or `0`.

* Use portable code constructs, that is code which will compile correctly under gcc and VisualStudio.

* Use shared_ptr, unique_ptr, and weak_ptr for dynamically allocated classes.

## Warnings

* We expect to have no compiler warnings.

# Std Library use

* Prefer `std::vector` over all other containers, "When choosing a container, remember vector is best;
Leave a comment to explain if you choose from the rest!" [C++20 standard](https://timsong-cpp.github.io/cppwp/n4861/sequence.reqmts#2). 

## Smart Pointers

Smart Pointers are a very useful feature of C++11, but sometimes we use them too much to manage memory, while this is correct this may cause of an overuse, especially when needing to pass objects into other functions. When smart pointers are used to indicate *ownership* things become somewhat clearer and it is easier to decide on how to pass the given object. See the [CPP Core Guidelines Resource Management](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-resource) especially R.20 - R.37 for more information

- Use smart pointer types to indicate owner ship 
  
  - `unique_ptr` expresses single ownership, the holder lifetime determines the owned objects lifetime
  - `shared_ptr` expresses shared ownership, the lifetime of the held object is the lifetime of all holding objects
  - `weak_ptr` expresses no ownership it is only an observer of an object with shared ownership, use it to break cycles of `shared_ptr`

Under this aspect, use: 

- `unique_ptr` and `shared_ptr` to indicate ownership
- Take smart pointers as parameters only to indicate lifetime semantics
    - A `Widget&` or `Widget*` is non owning, use when no transfer of ownership is occuring 
    - Take a `unique_ptr<Widget>` to express that a function assumes ownership of the `widget`
    - Take a `unique_ptr<Widget>&` to express that the function reseats the `widget`
    - Take a `shared_ptr<Widget>` to express that the function is part owner
    - Take a `shared_ptr<Widget>&` to express that the function might reseat the shared pointer

# Files

## Source files

* C++ header files should have the extension .h. Source files should have the extension .cpp, files defining inlined functions and templates should end in -inl.h

* Don't leave code in the header unless it is intended for inlining. In general there is no need to inline functions. Inline only when they are small, say, 10 lines or fewer. See [Google Style/Inline Functions](https://google.github.io/styleguide/cppguide.html#Inline_Functions). 

* A class should be declared in a header file and defined in a source file where the name of the files match the name of the class.
    * Filenames can freely use mixed case. Don't create file names (or class names) that differ only in capitalization. When in doubt, the file name should start with upper case.

* Unless absolutely necessary, have one .h file and one .cpp *or* -inl.h file per class, and only one class in each file.

* Don't put any implementations after the class body.

## Include files and `#include` statements

* Header files must  `#pragma once` as include guard.

* Include statements must be located at the top of a file only.
    * Except -inl.h files should be included at the end of the .h file.
* Include statements should be sorted and grouped so items are easy to find.
    * A typical approach:
        * First the .h file with the same name as the .cpp, followed by a blank line.
        * Then the includes from outside of iMSTK, in alphabetical order, followed by a blank line.
            * Only system header should be included via <>
        * Then the includes from iMSTK, in alphabetical order, followed by a blank line.
            * Any headers included from iMSTK should use "".

Example:
 ```   
#include "MyClass.h"

#include <iomanip>

#include "vtkActor.h"
#include "vtkRenderer.h"

#include "MainWindow.h"
#include "PropertiesDialog.h"
```

* When possible use forward class references but don't use forward references to other projects this may cause unintneded side-effects see https://google.github.io/styleguide/cppguide.html#Forward_Declarations

# Git Repository

* Git commit messages should follow the convention described [here](http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html). Basically:

```
Short one line description, 50 characters or less

A blank line, followed by a detailed description. The longer description
should be wrapped at 72 characters.
```

# Message Level Semantics

- `DEBUG`, Use at your discretion.
- `INFO`, Informational, notify of state changes. Example a file was successfully loaded
- `WARNING`, Something failed, but the impact of the failure is not know or minimal (e.g. purely visual). Example: The iteration limit was exceeded
- `SEVERE`, Something failed and will impact functionality, some parts of the program will not function correctly. Example: NaN was found in a calculation result, where none was ever expected
- `FATAL`, Used by assertion, after using this level the program will not be functional at all. Example a nullptr was passed where not null was expeced

# Error Handling

A function that fails to do its job due to invalid input parameters or a violation of constraints should not do so silently, just prining a warning is not sufficient

Prefer to use `CHECK()` or `LOG(FATAL)` and abort in most cases, a CHECK as per glog definition is a contract, if the contract is violated the programm will not be able to execute correctly. There is no point in proceeding, examples here are, nullptr passed where that is not acceptable if you think that the error should not force the system to shut down, use `LOG(WARNING)` but in this case the function that failed needs to provide a method that indicates to the user that a failure occured (e.g. returning `false`). Note that the latter will more likely cause more code to be written as now return values will have to be checked.
















