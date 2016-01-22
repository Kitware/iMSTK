###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

imstk_add_library(Collision
  SOURCES
    CollisionMoller.cpp
    MeshCollisionModel.cpp
    MeshToMeshCollision.cpp
    OctreeCell.cpp
    PlaneCollisionModel.cpp
    PlaneToMeshCollision.cpp
    SpatialHashCollision.cpp
  PUBLIC_HEADERS
    CollisionMoller.h
    MeshCollisionModel.h
    MeshToMeshCollision.h
    OctreeCell.h
    PlaneCollisionModel.h
    PlaneToMeshCollision.h
    SpatialHashCollision.h
    SurfaceTree.h
    SurfaceTree.hpp
    SurfaceTreeCell.h
    SurfaceTreeIterator.h
)

target_link_libraries(Collision
  PRIVATE
    Core
    Event
    Mesh
    Geometry
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
