#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( DataStructures
  H_FILES
    imstkGraph.h
    imstkGridBasedNeighborSearch.h
    imstkLooseOctree.h
    imstkNeighborSearch.h
    imstkSpatialHashTable.h
    imstkSpatialHashTableSeparateChaining.h
    imstkUniformSpatialGrid.h
  CPP_FILES
    imstkGraph.cpp
    imstkGridBasedNeighborSearch.cpp
    imstkLooseOctree.cpp
    imstkNeighborSearch.cpp
    imstkSpatialHashTable.cpp
    imstkSpatialHashTableSeparateChaining.cpp
  DEPENDS
    Common
    Geometry #TODO remove this dependency
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
