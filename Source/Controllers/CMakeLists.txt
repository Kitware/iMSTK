#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Controllers
  H_FILES
    imstkCameraController.h
    imstkDeviceControl.h
    imstkKeyboardControl.h
    imstkLaparoscopicToolController.h
    imstkMouseControl.h
    imstkRigidObjectController.h
    imstkSceneObjectController.h
    imstkTrackingDeviceControl.h
  CPP_FILES
    imstkCameraController.cpp
    imstkDeviceControl.cpp
    imstkKeyboardControl.cpp
    imstkLaparoscopicToolController.cpp
    imstkMouseControl.cpp
    imstkRigidObjectController.cpp
    imstkSceneObjectController.cpp
    imstkTrackingDeviceControl.cpp
  DEPENDS
	Devices
    SceneEntities
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
