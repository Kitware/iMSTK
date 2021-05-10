include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(modIMSTK.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(modIMSTK.i PROPERTIES SWIG_MODULE_NAME modIMSTK)

swig_add_library(modIMSTK
    TYPE SHARED
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/Source/csIMSTK"
    LANGUAGE csharp
    SOURCES modIMSTK.i)

set_target_properties(modIMSTK PROPERTIES SWIG_COMPILE_OPTIONS "-namespace;imstk")

target_link_libraries(modIMSTK PUBLIC SimulationManager)

