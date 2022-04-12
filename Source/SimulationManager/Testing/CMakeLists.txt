include(imstkAddTest)
if (iMSTK_USE_RENDERING_VTK)
  imstk_add_test( SimulationManager )
  # Note: Links to ViewerVTK for rendering support in tests
  target_link_libraries(SimulationManagerTests SimulationManager ViewerVTK)
endif()