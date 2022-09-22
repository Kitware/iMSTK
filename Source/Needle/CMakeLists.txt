#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(H_FILES
  imstkArcNeedle.h
  imstkNeedle.h
  imstkPuncture.h
  imstkPuncturable.h
  imstkStraightNeedle.h
  )

set(SRC_FILES
  imstkNeedle.cpp
  imstkPuncturable.cpp
  imstkStraightNeedle.cpp
  )

include(imstkAddLibrary)
imstk_add_library(Needle
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    ComponentModel
    Geometry
    DynamicalModels
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()