include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include(UseSWIG)

set_source_files_properties(SwigInterface/imstkCWrapper.i PROPERTIES CPLUSPLUS ON)

swig_add_library(iMSTKCWrapper
    TYPE SHARED
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/Source/Wrappers/csharp"
    LANGUAGE csharp
    SOURCES SwigInterface/imstkCWrapper.i)

set_target_properties(iMSTKCWrapper PROPERTIES SWIG_COMPILE_OPTIONS "-namespace;imstk")
if(iMSTK_SWIG_PINNED_ARRAY)
    set_target_properties(iMSTKCWrapper PROPERTIES SWIG_COMPILE_DEFINITIONS "SWIG_PINNED_ARRAY")
endif()
if(iMSTK_USE_OpenHaptics)
    set_target_properties(iMSTKCWrapper PROPERTIES SWIG_COMPILE_DEFINITIONS "iMSTK_USE_OpenHaptics")
	target_compile_definitions(iMSTKCWrapper PRIVATE iMSTK_USE_OpenHaptics)
endif()

target_link_libraries(iMSTKCWrapper PUBLIC SimulationManager Filtering)
target_compile_options(iMSTKCWrapper PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wno-unused-function>
    $<$<CXX_COMPILER_ID:MSVC>:
        -W4 -MP -wd4505 /bigobj>)

# Under MSVC we can generate a project for the examples
if (MSVC)
  add_subdirectory(iMSTKCSharp)
  if (${IMSTK_WRAPPER_BUILT})
    add_subdirectory(Examples)
  endif()
endif()

#-----------------------------------------------------------------------------
# Install library
#-----------------------------------------------------------------------------
install( TARGETS iMSTKCWrapper EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib COMPONENT Development)

if (LINUX)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink iMSTKCWrapper.so ${CMAKE_INSTALL_PREFIX}/lib/libiMSTKCWrapper.so)")
endif()

# Install all ".cs" files
install(
    DIRECTORY "${CMAKE_BINARY_DIR}/Source/Wrappers/csharp/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iMSTKSharp/"
    FILES_MATCHING PATTERN "*.cs")
