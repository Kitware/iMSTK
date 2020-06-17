#-----------------------------------------------------------------------------
# Add Pulse External Project
#-----------------------------------------------------------------------------

include(imstkAddExternalProject)
imstk_add_external_project( Pulse
  GIT_REPOSITORY https://gitlab.kitware.com/physiology/engine.git
  GIT_TAG 3.x
  GIT_SHALLOW TRUE
  # URL https://gitlab.kitware.com/physiology/engine/-/archive/3.x/archive.zip
  # URL_MD5 cf8edc032dc473668c6b3109cfee19e8
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
