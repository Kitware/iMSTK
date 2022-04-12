#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Geometry
  H_FILES
    Analytic/imstkAnalyticalGeometry.h
    Analytic/imstkCapsule.h
    Analytic/imstkCylinder.h
    Analytic/imstkOrientedBox.h
    Analytic/imstkPlane.h
    Analytic/imstkSphere.h
    Implicit/imstkCompositeImplicitGeometry.h
    Implicit/imstkImplicitFunctionFiniteDifferenceFunctor.h
    Implicit/imstkImplicitGeometry.h
    Implicit/imstkSignedDistanceField.h
    Mesh/imstkHexahedralMesh.h
    Mesh/imstkImageData.h
    Mesh/imstkLineMesh.h
    Mesh/imstkPointSet.h
    Mesh/imstkSurfaceMesh.h
    Mesh/imstkTetrahedralMesh.h
    Mesh/imstkVolumetricMesh.h
    Particles/imstkRenderParticles.h
    imstkGeometry.h
    imstkGeometryUtilities.h
  CPP_FILES
    Analytic/imstkAnalyticalGeometry.cpp
    Analytic/imstkCapsule.cpp
    Analytic/imstkCylinder.cpp
    Analytic/imstkOrientedBox.cpp
    Analytic/imstkPlane.cpp
    Analytic/imstkSphere.cpp
    Implicit/imstkCompositeImplicitGeometry.cpp
    Implicit/imstkSignedDistanceField.cpp
    Mesh/imstkHexahedralMesh.cpp
    Mesh/imstkImageData.cpp
    Mesh/imstkLineMesh.cpp
    Mesh/imstkPointSet.cpp
    Mesh/imstkSurfaceMesh.cpp
    Mesh/imstkTetrahedralMesh.cpp
    Particles/imstkRenderParticles.cpp
    imstkGeometry.cpp
    imstkGeometryUtilities.cpp
  DEPENDS
    Common
    VTK::CommonDataModel
    VTK::FiltersSources
    VTK::FiltersGeneral
    )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
