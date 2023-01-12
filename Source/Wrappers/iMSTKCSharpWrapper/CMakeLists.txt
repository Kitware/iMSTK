cmake_minimum_required(VERSION 3.12)

project(iMSTKSharp VERSION 0.1.0 LANGUAGES CSharp C CXX)

# \todo: The iMSTKSharp target is copied by visual studio (not cmake) into the Examples build directory
# When iMSTKSharp is modified this target is not reinstalled/copied. This trips up visual studios and
# especially intellisense

# Create inlcude file with definitions so we can detect imstk build
# configurations on the source side, capitalize all symbols as well
# Do this before the glob so that the new file will be found
if(iMSTK_USE_OpenHaptics)
    set(IMSTK_USE_OPENHAPTICS ON)
endif()
if(iMSTK_USE_VRPN)
    set(IMSTK_USE_VRPN ON)
endif()

# Get all the csharp files from the output directory
file(GLOB SRC_FILES CONFIGURE_DEPENDS "${SWIG_OUTPUT_DIR}/*.cs")

find_package(iMSTKCWrapper REQUIRED)

add_library(iMSTKSharp SHARED ${SRC_FILES})

if (iMSTK_SWIG_PINNED_ARRAY)
    target_compile_options(iMSTKSharp PRIVATE "/unsafe")
endif()

set_property(TARGET iMSTKSharp PROPERTY VS_DOTNET_REFERENCES "System")

install(TARGETS iMSTKSharp EXPORT iMSTKSharpTargets
	RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
	LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
	ARCHIVE DESTINATION lib COMPONENT Development)

source_group("Source" FILES ${SRC_FILES})

# Add example targets
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Examples)