#-----------------------------------------------------------------------------
# Compile Shaders
#-----------------------------------------------------------------------------

add_custom_target(VulkanShaders ALL)
function(compileShaders sourceShader binaryShader)
  add_custom_command(
    TARGET VulkanShaders
    COMMAND ${CMAKE_INSTALL_PREFIX}/bin/glslangvalidator -V ${Vulkan_Shaders}/${sourceShader} -o ${PROJECT_BINARY_DIR}/Shaders/VulkanShaders/${binaryShader})
endfunction()

function(CopyAndCompileShaders)
  if( iMSTK_USE_Vulkan )
  
    set(Vulkan_Shaders "${CMAKE_SOURCE_DIR}/Source/Rendering/VulkanRenderer/VulkanShaders")
    
    file(MAKE_DIRECTORY  "${PROJECT_BINARY_DIR}/Shaders/VulkanShaders/mesh")
    # Mesh shaders
    compileShaders(mesh/mesh_vert.vert mesh/mesh_vert.spv)
    compileShaders(mesh/mesh_tesc.tesc mesh/mesh_tesc.spv)
    compileShaders(mesh/mesh_tese.tese mesh/mesh_tese.spv)
    compileShaders(mesh/mesh_frag.frag mesh/mesh_frag.spv)
    compileShaders(mesh/decal_vert.vert mesh/decal_vert.spv)
    compileShaders(mesh/decal_frag.frag mesh/decal_frag.spv)
    compileShaders(mesh/particle_vert.vert mesh/particle_vert.spv)
    compileShaders(mesh/particle_frag.frag mesh/particle_frag.spv)
    compileShaders(mesh/shadow_vert.vert mesh/shadow_vert.spv)
    compileShaders(mesh/shadow_frag.frag mesh/shadow_frag.spv)
    compileShaders(mesh/depth_frag.frag mesh/depth_frag.spv)

    file(MAKE_DIRECTORY  "${PROJECT_BINARY_DIR}/Shaders/VulkanShaders/PostProcessing")
    # Post processing shaders
    compileShaders(PostProcessing/HDR_tonemap_frag.frag PostProcessing/HDR_tonemap_frag.spv)
    compileShaders(PostProcessing/postprocess_vert.vert PostProcessing/postprocess_vert.spv)
    compileShaders(PostProcessing/postprocess_frag.frag PostProcessing/postprocess_frag.spv)
    compileShaders(PostProcessing/passthrough_frag.frag PostProcessing/passthrough_frag.spv)
    compileShaders(PostProcessing/sss_frag.frag PostProcessing/sss_frag.spv)
    compileShaders(PostProcessing/lens_distortion_frag.frag PostProcessing/lens_distortion_frag.spv)
    compileShaders(PostProcessing/composite_frag.frag PostProcessing/composite_frag.spv)
    compileShaders(PostProcessing/vr_composite_frag.frag PostProcessing/vr_composite_frag.spv)
    compileShaders(PostProcessing/bloom_threshold_frag.frag PostProcessing/bloom_threshold_frag.spv)
    compileShaders(PostProcessing/blur_horizontal_frag.frag PostProcessing/blur_horizontal_frag.spv)
    compileShaders(PostProcessing/blur_vertical_frag.frag PostProcessing/blur_vertical_frag.spv)
    compileShaders(PostProcessing/bloom_threshold_frag.frag PostProcessing/bloom_threshold_frag.spv)
    compileShaders(PostProcessing/ao_frag.frag PostProcessing/ao_frag.spv)
    compileShaders(PostProcessing/bilateral_blur_horizontal_frag.frag PostProcessing/bilateral_blur_horizontal_frag.spv)
    compileShaders(PostProcessing/bilateral_blur_vertical_frag.frag PostProcessing/bilateral_blur_vertical_frag.spv)
    compileShaders(PostProcessing/depth_downscale_frag.frag PostProcessing/depth_downscale_frag.spv)

    install(DIRECTORY ${PROJECT_BINARY_DIR}/Shaders/VulkanShaders
      DESTINATION ${CMAKE_INSTALL_PREFIX}/data/shaders)
  
  endif()
endfunction()