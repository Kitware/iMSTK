include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(imstkCWrapper.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(imstkCWrapper.i PROPERTIES SWIG_MODULE_NAME imstkCWrapper)

swig_add_library(imstkCWrapper
    TYPE SHARED
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/Source/CSWrapper"
    LANGUAGE csharp
    SOURCES imstkCWrapper.i)

set_target_properties(imstkCWrapper PROPERTIES SWIG_COMPILE_OPTIONS "-namespace;imstk")
if(iMSTK_USE_OpenHaptics)
    set_target_properties(imstkCWrapper PROPERTIES SWIG_COMPILE_DEFINITIONS "iMSTK_USE_OpenHaptics")
	target_compile_definitions(imstkCWrapper PRIVATE iMSTK_USE_OpenHaptics)
endif()

target_link_libraries(imstkCWrapper PUBLIC SimulationManager Filtering)

#-----------------------------------------------------------------------------
# Install library
#-----------------------------------------------------------------------------
install( TARGETS imstkCWrapper EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib COMPONENT Development)

# Install all ".cs" files
install(
    DIRECTORY "${CMAKE_BINARY_DIR}/Source/CSWrapper/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iMSTKSharp/"
    FILES_MATCHING PATTERN "${CMAKE_BINARY_DIR}/Source/CSWrapper/*.cs")
