/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkGeometry.h"
#include "imstkMath.h"

namespace imstk
{
class CollidingObject;
class FEMDeformableBodyModel;
class Graph;
template<typename Matrix>
class NonLinearSystem;
class PointSet;
class Scene;
class SceneManager;
class SceneObject;
class SurfaceMesh;
class TetrahedralMesh;

namespace apiutils
{
///
/// \brief Create a analytical visual scene object that and add it to the scene
///
std::shared_ptr<SceneObject> createVisualAnalyticalSceneObject(Geometry::Type type,
                                                               std::shared_ptr<Scene> scene,
                                                               const std::string& objName,
                                                               const Vec3d scale = Vec3d(1.0, 1.0, 1.0),
                                                               const Vec3d t      = Vec3d(0., 0., 0.));

///
/// \brief Create a analytical colliding scene object that and add it to the scene
///
std::shared_ptr<CollidingObject> createCollidingAnalyticalSceneObject(Geometry::Type type,
                                                                      std::shared_ptr<Scene> scene,
                                                                      const std::string& objName,
                                                                      const Vec3d scale = Vec3d(1.0, 1.0, 1.0),
                                                                      const Vec3d t      = Vec3d(0., 0., 0.));

///
/// \brief Read a mesh, create a visual scene object and add to the scene
///
std::shared_ptr<SceneObject> createAndAddVisualSceneObject(std::shared_ptr<Scene> scene,
                                                           const std::string&     fileName,
                                                           const std::string&     objectName);

///
/// \brief Create a non-linear system using FEM dynamic model
///
std::shared_ptr<NonLinearSystem<SparseMatrixd>> createNonLinearSystem(std::shared_ptr<FEMDeformableBodyModel> dynaModel);

///
/// \brief Create a \ref Graph ref
///
std::shared_ptr<Graph> getMeshGraph(std::shared_ptr<PointSet> m);
std::shared_ptr<Graph> getMeshGraph(std::shared_ptr<SurfaceMesh> m);
std::shared_ptr<Graph> getMeshGraph(std::shared_ptr<TetrahedralMesh> m);
} //apiutils
} // imstk
