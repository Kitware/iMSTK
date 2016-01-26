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

imstk_add_library(Core
  SOURCES
    BaseMesh.cpp
    CollisionConfig.cpp
    CollisionDetection.cpp
    CollisionManager.cpp
    Color.cpp
    ContactHandling.cpp
    CoreClass.cpp
    Dispatcher.cpp
    ErrorLog.cpp
    Event.cpp
    EventHandler.cpp
    Geometry.cpp
    IOStream.cpp
    Light.cpp
    Model.cpp
    Module.cpp
    RenderDetail.cpp
    Scene.cpp
    Timer.cpp
    UnifiedId.cpp
  PUBLIC_HEADERS
	BaseMesh.h
    CollisionConfig.h
    CollisionDetection.h
    CollisionManager.h
    CollisionModelIterator.h
    CollisionManager.h
    Color.h
    Config.h
    ConfigRendering.h
    ContactHandling.h
    CoreClass.h
    DataStructures.h
    DataStructures.hpp
    Dispatcher.h
    DisplayText.h
    ErrorLog.h
    EventData.h
    Event.h
    EventHandler.h
    Factory.h
    Factory.hpp
    Geometry.h
    IOStream.h
    Light.h
    MakeUnique.h
    Matrix.h
    MemoryBlock.h
    Model.h
    Module.h
    Quaternion.h
    RenderDelegate.h
    RenderDetail.h
    Scene.h
    Texture.h
    Timer.h
    UnifiedId.h
    Utils.h
    Vector.h
    WorkerThread.h
)

target_link_libraries(Core
  PUBLIC
    ${OPENGL_LIBRARY}
    sfml::graphics
    VegaFEM::objMesh
    VegaFEM::volumetricMesh
    vtkglew
)

if(UNIX)
  target_compile_features(Core PUBLIC cxx_auto_type)
endif()

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
