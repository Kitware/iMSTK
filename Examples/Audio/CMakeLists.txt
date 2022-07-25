###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################


project(Example-Audio)

#-----------------------------------------------------------------------------
# Create executable
#-----------------------------------------------------------------------------
imstk_add_executable(${PROJECT_NAME} AudioExample.cpp)

#-----------------------------------------------------------------------------
# Add the target to Examples folder
#-----------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples)

#-----------------------------------------------------------------------------
# Link libraries to executable
#-----------------------------------------------------------------------------
if(UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  target_link_libraries(${PROJECT_NAME}
    Common
    Threads::Threads)
else()
  target_link_libraries(${PROJECT_NAME}
    Common
    ${SFML_LIBRARIES})
endif()