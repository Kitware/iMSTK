#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
if(iMSTK_USE_Pulse)
  include(imstkAddLibrary)
  imstk_add_library( Pulse
    DEPENDS
      Scene
      Pulse::CommonPulseModels)
endif()