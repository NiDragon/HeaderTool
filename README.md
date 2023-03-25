# HeaderTool - RTTR Bindings Generator for C++

Purpose of this project is to parse C++ header files and Generate C++ reflection bindings for [LibRttr](https://www.rttr.org/doc/rttr-0-9-6/classes.html).
-
This code is provided under the [MIT-License](LICENSE).
___
This project was created for personal use I figured since I saw nothing on existing ways to do exactly what I wanted I created one. There is absolute no guarntee this will be useful to anyone or not destructive to your code always keep backups and test properly. I deem that it is production ready but that depends on your specific needs.

> Drawbacks: No way to implement class inheritence without manually inserting it into the class body. However, since this just generates an external header with the expected code that is not possible. Meaning RTTR_ENABLE() is still required. Also requires LibClang a big library for such small code.

## Requirements
* LibClang
* C++17 Compiler Support

## How To Build
> For Windows
> * Create clang directory in the root
> * Create subdirectories include and lib
> * Copy libclang to clang/lib
> * Copy clang and clang-c folders to clang/include
> For Linux
> * sudo apt install llvm
> * sudo apt install cmake
> ___
> cmake -B ./Build
> cd Build
> cmake --build . --config Release

## Command Line Arguments
```
-f <filename>       - Source filename to be parsed.
-i <path>           - Include directory to resolve references.
-m <module name>    - Module name for class path generation.
-o <output path>    - Directory to write generated output to.
-t <timestamp path> - Directory to write timestamps to.
-hotswap            - Parse reflection info as plugin for hot reload.
-help               - Shows this information.
```

## Supported Macros:
* CLASS(...)
* PROPERTY(...)
* FUNCTION(...)
* ENUM(...)

## Special Tags
* Access (Public, Private, Protected)
> Default Access=Public
* Policy (Behavior depends on type supported values - AsPointer, Discard, AsRef)
> Policy=Discard only applies to functions
* DefaultArgs (Only applies to functions)

### Usage:
> HeaderTool -f "Vector.h" -hotswap

### Example: Vector.h
```cpp
// Define macros for reflection generation
#define CLASS(...)
#define ENUM(...)
#define FUNCTION(...)

// When used on function to make property accessor
// the macro must be above a function prefixed with Get 
// needs to also have a matching function prefixed with Set
#define PROPERTY(...)

CLASS(Description="A Three Component Vector")
class Vector3 {
public:
    Vector3();

    // This creates a function with a reference return value
    FUNCTION(Description="Get Vector Name If Any",Policy=AsRef)
    std::string& GetVectorName();
	
    // Access=Public is not necessary because all access defaults to Public other options are Private or Protected
    FUNCTION(Description="Set Vector XYZ",DefaultArgs=[1.0f, 1.0f, 1.0f],Access=Public)
    void SetXYZ(float x, float y, float z);

    // This creates an accessor
    PROPERTY(Description="Vector3 X Component") 
    const float& GetX();
    void SetX(const float& x);

    PROPERTY(Description = "Vector3 Y Component")
    const float& GetY();
    void SetY(const float& y);

    PROPERTY(Description = "Vector3 Z Component")
    const float& GetZ();
    void SetZ(const float& z);

    // More for debug reasons
    std::string name;

    float x; 
    float y;
    float z;
};

// This auto generates names bindings for enum values
ENUM(Description="Describes A Direction")
enum Direction
{
	UP,
	DOWN,
	LEFT,
	RIGHT
}
```

### Output: Vector.generated.h
```cpp
#include <rttr/registration.h>

#if !defined(_VECTOR_GENERATED_H_)
#define _VECTOR_GENERATED_H_

namespace _REFLECTION_VECTOR_GENERATED_H_ {
    RTTR_PLUGIN_REGISTRATION
    {
        rttr::registration::class_<Vector3>("Vector3")
        (
            rttr::metadata("Description", rttr::string_view("A Three Component Vector"))
        )
        .constructor<>()
        .method("GetVectorName", &Vector3::GetVectorName)
        (
            rttr::metadata("Description", rttr::string_view("Get Vector Name If Any"))
        )
        .method("SetXYZ", &Vector3::SetXYZ)
        (
            rttr::parameter_names("x", "y", "z"), 
            rttr::default_arguments(1.0f,1.0f,1.0f),
            rttr::metadata("Description", rttr::string_view("Set Vector XYZ"))
        )
        .property("X", &Vector3::GetX, &Vector3::SetX)
        (
            rttr::metadata("Description", rttr::string_view("Vector3 X Component"))
        )
        .property("Y", &Vector3::GetY, &Vector3::SetY)
        (
            rttr::metadata("Description", rttr::string_view("Vector3 Y Component"))
        )
        .property("Z", &Vector3::GetZ, &Vector3::SetZ)
        (
            rttr::metadata("Description", rttr::string_view("Vector3 Z Component"))
        )
        ;
        rttr::registration::enumeration<Direction>("Direction")
        (
            rttr::value("DOWN", Direction::DOWN), 
            rttr::value("LEFT", Direction::LEFT), 
            rttr::value("RIGHT", Direction::RIGHT), 
            rttr::value("UP", Direction::UP), 
            rttr::metadata("Description", rttr::string_view("Describes A Direction"))
        )
        ;
    }
};

#endif
```

> ### Things that would be nice in the future:
> * Constructor Access Support
> * Constructor Binding Policy (eg. Create Raw Pointers of Types)
