#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
file(GLOB hFiles *.h)
file(GLOB cppFiles *cpp)

include(imstkAddLibrary)
imstk_add_library( FilteringCore
  H_FILES
    ${hFiles}
  CPP_FILES
    ${cppFiles}
  DEPENDS
    Geometry
    DataStructures
    ${VTK_LIBRARIES})
