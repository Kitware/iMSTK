#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(H_FILES
  Camera/imstkCamera.h
  Components/imstkAxesModel.h
  Components/imstkNeedle.h
  Components/imstkObjectControllerGhost.h
  Components/imstkVisualModel.h
  Components/imstkVertexLabelVisualModel.h
  Lights/imstkDirectionalLight.h
  Lights/imstkIBLProbe.h
  Lights/imstkLight.h
  Lights/imstkPointLight.h
  Lights/imstkSpotLight.h
  Loader/imstkVisualObjectImporter.h
  Objects/imstkAnimationModel.h
  Objects/imstkAnimationObject.h
  Objects/imstkCollidingObject.h
  Objects/imstkCollisionDataDebugObject.h
  Objects/imstkDebugGeometryObject.h
  Objects/imstkDynamicObject.h
  Objects/imstkFeDeformableObject.h
  Objects/imstkLevelSetDeformableObject.h
  Objects/imstkPbdObject.h
  Objects/imstkRigidObject2.h
  Objects/imstkSceneObject.h
  Objects/imstkSphObject.h
  )

set(SRC_FILES
  Camera/imstkCamera.cpp
  Components/imstkNeedle.cpp
  Components/imstkObjectControllerGhost.cpp
  Components/imstkVisualModel.cpp
  Lights/imstkIBLProbe.cpp
  Loader/imstkVisualObjectImporter.cpp
  Objects/imstkAnimationModel.cpp
  Objects/imstkAnimationObject.cpp
  Objects/imstkCollidingObject.cpp
  Objects/imstkCollisionDataDebugObject.cpp
  Objects/imstkDebugGeometryObject.cpp
  Objects/imstkDynamicObject.cpp
  Objects/imstkFeDeformableObject.cpp
  Objects/imstkLevelSetDeformableObject.cpp
  Objects/imstkPbdObject.cpp
  Objects/imstkRigidObject2.cpp
  Objects/imstkSceneObject.cpp
  Objects/imstkSphObject.cpp
  )

if (iMSTK_USE_MODEL_REDUCTION)
  list(APPEND H_FILES
    Objects/imstkReducedFeDeformableObject.h
    )
  list(APPEND SRC_FILES
    Objects/imstkReducedFeDeformableObject.cpp
    )
endif()

include(imstkAddLibrary)
imstk_add_library(SceneEntities
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    ComponentModel
    GeometryMappers
    Materials
    MeshIO
    Constraints
    Devices
    DynamicalModels
    CollisionDetection
    Controllers
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()