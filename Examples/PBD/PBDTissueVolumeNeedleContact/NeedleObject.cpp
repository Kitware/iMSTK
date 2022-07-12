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

#include "NeedleObject.h"
#include "imstkLineMesh.h"
#include "imstkVecDataArray.h"
#include "imstkMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkIsometricMap.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

using namespace imstk;

NeedleObject::NeedleObject(const std::string& name) : PbdObject(name)
{
    auto toolGeometry = std::make_shared<LineMesh>();
    auto verticesPtr  = std::make_shared<VecDataArray<double, 3>>(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    auto syringeMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Syringes/Disposable_Syringe.stl");
    syringeMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 4.4, 0.0), Geometry::TransformType::ApplyToData);
    syringeMesh->scale(0.0055, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 0.1, 0.0));

    setVisualGeometry(syringeMesh);
    setCollidingGeometry(toolGeometry);
    setPhysicsGeometry(toolGeometry);
    setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeometry, syringeMesh));
    getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);
}