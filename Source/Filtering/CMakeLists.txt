#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Filtering
  H_FILES
    imstkAppendMesh.h
    imstkCleanMesh.h
    imstkExtractEdges.h
    imstkFastMarch.h
    imstkImageDistanceTransform.h
    imstkImageGradient.h
    imstkImageResample.h
    imstkImageReslice.h
    imstkImplicitGeometryToImageData.h
    imstkLocalMarchingCubes.h
    imstkQuadricDecimate.h
    imstkSelectEnclosedPoints.h
    imstkSurfaceMeshCut.h
    imstkSurfaceMeshDistanceTransform.h
    imstkSurfaceMeshFlyingEdges.h
    imstkSurfaceMeshImageMask.h
    imstkSurfaceMeshSmoothen.h
    imstkSurfaceMeshSubdivide.h
    imstkSurfaceMeshTextureProject.h
  CPP_FILES
    imstkAppendMesh.cpp
    imstkCleanMesh.cpp
    imstkExtractEdges.cpp
    imstkFastMarch.cpp
    imstkImageDistanceTransform.cpp
    imstkImageGradient.cpp
    imstkImageResample.cpp
    imstkImageReslice.cpp
    imstkImplicitGeometryToImageData.cpp
    imstkLocalMarchingCubes.cpp
    imstkQuadricDecimate.cpp
    imstkSelectEnclosedPoints.cpp
    imstkSurfaceMeshCut.cpp
    imstkSurfaceMeshDistanceTransform.cpp
    imstkSurfaceMeshFlyingEdges.cpp
    imstkSurfaceMeshImageMask.cpp
    imstkSurfaceMeshSmoothen.cpp
    imstkSurfaceMeshSubdivide.cpp
    imstkSurfaceMeshTextureProject.cpp
  DEPENDS
    FilteringCore
    VTK::ImagingMath
    VTK::ImagingStencil
    VTK::FiltersExtraction
    VTK::FiltersGeneral
    VTK::FiltersModeling
    )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()