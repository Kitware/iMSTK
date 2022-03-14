# Add an example as its own target
function(add_example example_name sources)
  set(target "Example-${example_name}-CSharp")
  add_executable(${target} ${sources})
  set_property(TARGET ${target} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
  set_property(TARGET ${target} PROPERTY VS_DOTNET_REFERENCES "System;iMSTKSharp")
  set_property(TARGET ${target} PROPERTY FOLDER Examples/CSharp)

  if (MSVC)
    set(BIN_PATH "${CMAKE_INSTALL_PREFIX}/bin/")
    # csproj files picky about slashes
    string(REPLACE "/" "\\" BIN_PATH "${BIN_PATH}")
    set_property(TARGET ${target} PROPERTY VS_GLOBAL_StartWorkingDirectory ${BIN_PATH})

    set_target_properties (${target} PROPERTIES
        FOLDER Examples)
  endif()

  add_custom_command(TARGET ${target} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${CMAKE_INSTALL_PREFIX}/bin)
  
  add_dependencies(${target} iMSTKSharp)

  install(TARGETS ${target})
endfunction()

add_example(FemDeformable femDeformable.cs)
add_example(PBDCloth pbdCloth.cs)
add_example(PBDClothCollision pbdClothCollision.cs)
add_example(PBDCollisionOneObject pbdCollisionOneObject.cs)
add_example(PBDVolume pbdVolume.cs)
add_example(RigidBody2 rigidBody2.cs)
add_example(SPHFluid sphFluid.cs)
add_example(TestGeometry testGeometry.cs)

if (iMSTK_USE_OpenHaptics)
    add_example(PBDCutting pbdCutting.cs)
    add_example(SDFHaptics sdfHaptics.cs)
endif()