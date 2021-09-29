project(CSharpExamples LANGUAGES CSharp)

# Check https://stackoverflow.com/questions/52556785/code-generator-generating-its-own-cmake-files-and-targets/52714922#52714922
# https://stackoverflow.com/questions/55713475/correct-use-of-vs-debugger-working-directory-etc

function(add_example example_name sources)
  
  set(target "Example-${example_name}-CSharp")
  add_executable(${target} ${sources})
  set_property(TARGET ${target} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
  set_property(TARGET ${target} PROPERTY VS_DOTNET_REFERENCES "System;iMSTKCSharp")
  set_property(TARGET ${target} PROPERTY FOLDER Examples/CSharp)

  if (MSVC)
    set(BIN_PATH "${CMAKE_INSTALL_PREFIX}/bin/")
    # csproj files picky about slashes
    string(REPLACE "/" "\\" BIN_PATH "${BIN_PATH}")
    set_property(TARGET ${target} PROPERTY VS_GLOBAL_StartWorkingDirectory ${BIN_PATH})
  endif()

  add_custom_command(TARGET ${target} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${CMAKE_INSTALL_PREFIX}/bin)
  
  add_dependencies(${target} iMSTKCSharp)
  
  target_link_libraries(${target} iMSTKCWrapper)
endfunction()

message (WARNING "Adding CSharp Examples, please note to run the examples from visual studio you need to set the working directory in the debug project properties to ${CMAKE_INSTALL_PREFIX}/bin")

add_example(FemDeformable femDeformable.cs)
add_example(PBDCloth pbdCloth.cs)
add_example(PBDClothCollision pbdClothCollision.cs)
add_example(PBDCollisionOneObject pbdCollisionOneObject.cs)
add_example(PBDCutting pbdCutting.cs)
add_example(PBDVolume pbdVolume.cs)
add_example(RigidBody2 rigidBody2.cs)
add_example(SDFHaptics sdfHaptics.cs)
add_example(SPHFluid sphFluid.cs)
add_example(TestGeometry testGeometry.cs)