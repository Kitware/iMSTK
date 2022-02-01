cmake_minimum_required(VERSION 3.9)

project(iMSTKSharp VERSION 0.1.0 LANGUAGES CSharp C CXX)

# \todo: The iMSTKSharp target is copied by visual studios (not cmake) into the Examples build directory
# When iMSTKSharp is modified this target is not reinstalled/copied. This trips up visual studios and
# especially intellisense

# Get all the csharp files from the output directory
file(GLOB SRC_FILES "${SWIG_OUTPUT_DIR}/*.cs")

find_package(iMSTKCWrapper REQUIRED)

add_library(iMSTKSharp SHARED ${SRC_FILES})

if (iMSTK_SWIG_PINNED_ARRAY)
    target_compile_options(iMSTKSharp PRIVATE "/unsafe")
endif()

set_property(TARGET iMSTKSharp PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
set_property(TARGET iMSTKSharp PROPERTY VS_DOTNET_REFERENCES "System")

install(TARGETS iMSTKSharp EXPORT iMSTKSharpTargets
	RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
	LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
	ARCHIVE DESTINATION lib COMPONENT Development)

source_group("Source" FILES ${SRC_FILES})

# Add example targets
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Examples)