  include(imstkAddTest)
  imstk_add_visual_test( Rendering )
  target_link_libraries( RenderingVisualTests Common SimulationManager)