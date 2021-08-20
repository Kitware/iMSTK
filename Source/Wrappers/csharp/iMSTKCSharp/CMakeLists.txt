project(iMSTKCSharp VERSION 0.1.0 LANGUAGES CSharp)

# Check https://stackoverflow.com/questions/52556785/code-generator-generating-its-own-cmake-files-and-targets/52714922#52714922

file(GLOB csharp_files "${CMAKE_INSTALL_PREFIX}/include/iMSTKSharp/*.cs")

if (csharp_files)
	set(IMSTK_WRAPPER_BUILT ON CACHE INTERNAL "")
	add_library(${PROJECT_NAME} SHARED ${csharp_files})
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCES "System")
else ()
	set(IMSTK_WRAPPER_BUILT OFF CACHE INTERNAL "")
	message(WARNING "C# files not found, rerun configure after building the swig iMSTKCWrapper")
endif()