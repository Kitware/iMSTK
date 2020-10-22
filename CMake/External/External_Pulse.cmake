#-----------------------------------------------------------------------------
# Add Pulse External Project
#-----------------------------------------------------------------------------

include(imstkAddExternalProject)
imstk_add_external_project( Pulse
  GIT_REPOSITORY https://gitlab.kitware.com/physiology/engine.git
  GIT_TAG f4d501c2f7cc091dc053a96ab5a56f79b7528c4f
  GIT_SHALLOW TRUE
  #URL https://gitlab.kitware.com/physiology/engine/-/archive/3.x/archive.zip
  #URL_HASH 4189591a6859df46920bbc35f160eef23fc33ffd
  CMAKE_CACHE_ARGS
    -DPulse_SLAVE_BUILD:BOOL=ON
    -DPulse_JAVA_API:BOOL=OFF
    -DPulse_PYTHON_API:BOOL=OFF
  INSTALL_COMMAND $(SKIP_STEP_COMMAND)
)

if(NOT USE_SYSTEM_Pulse)
  set(Pulse_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/Pulse-3.0)
  #message(STATUS "Pulse_DIR : ${Pulse_DIR}")
endif()
