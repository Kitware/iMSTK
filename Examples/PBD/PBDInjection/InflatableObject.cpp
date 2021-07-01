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

#include "InflatableObject.h"
#include "imstkNew.h"
#include "imstkVecDataArray.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkCollisionUtils.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkParallelFor.h"
#include "imstkPbdConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkTexture.h"
#include "imstkVisualModel.h"
#include "imstkCollisionUtils.h"

#include "imstkPbdInflatableDistanceConstraint.h"
#include "imstkPbdInflatableVolumeConstraint.h"

InflatableObject::InflatableObject(const std::string& name, const Vec3d& tissueSize, const Vec3i& tissueDim, const Vec3d& tissueCenter) : PbdObject(name)
{
    // Setup the Geometry
    makeTetGrid(tissueSize, tissueDim, tissueCenter);
    m_objectSurfMesh = m_objectTetMesh->extractSurfaceMesh();
    m_objectSurfMesh->flipNormals();
    //setXZPlaneTexCoords(4.0);
    setSphereTexCoords(4.0);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.1;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 2;
    pbdParams->m_viscousDampingCoeff = 0.05;

    // Fix the borders
    for (int z = 0; z < tissueDim[2]; z++)
    {
        for (int y = 0; y < tissueDim[1]; y++)
        {
            for (int x = 0; x < tissueDim[0]; x++)
            {
                if (x == 0 || z == 0 || x == tissueDim[0] - 1 || z == tissueDim[2] - 1 || y == 0)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + tissueDim[0] * (y + tissueDim[1] * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(m_objectTetMesh);
    pbdModel->configure(pbdParams);

    auto distanceFunctor = std::make_shared<PbdInflatableDistanceConstraintFunctor>();
    distanceFunctor->setStiffness(0.95);
    auto volumeFunctor = std::make_shared<PbdInflatableVolumeConstraintFunctor>();
    volumeFunctor->setStiffness(0.9);
    pbdModel->addPbdConstraintFunctor(distanceFunctor);
    pbdModel->addPbdConstraintFunctor(volumeFunctor);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel(m_objectSurfMesh);
    visualModel->setRenderMaterial(material);
    addVisualModel(visualModel);

    // Add a visual model to render the normals of the surface
    imstkNew<VisualModel> normalsVisualModel(m_objectSurfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);
    addVisualModel(normalsVisualModel);

    // Setup the Object
    setPhysicsGeometry(m_objectTetMesh);
    setCollidingGeometry(m_objectSurfMesh);
    setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(m_objectTetMesh, m_objectSurfMesh));
    setDynamicalModel(pbdModel);
}

bool
InflatableObject::initialize()
{
    PbdObject::initialize();

    m_constraintContainer = m_pbdModel->getConstraints();

    return true;
}

void
InflatableObject::makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<TetrahedralMesh> tissueMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();

    const Vec3d dx = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                vertices[x + dim[0] * (y + dim[1] * z)] = Vec3i(x, y, z).cast<double>().cwiseProduct(dx) - size * 0.5 + center;
            }
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 4>> indicesPtr;
    VecDataArray<int, 4>&          indices = *indicesPtr.get();
    for (int z = 0; z < dim[2] - 1; z++)
    {
        for (int y = 0; y < dim[1] - 1; y++)
        {
            for (int x = 0; x < dim[0] - 1; x++)
            {
                int cubeIndices[8] =
                {
                    x + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * (z + 1))
                };

                // Alternate the pattern so the edges line up on the sides of each voxel
                if ((z % 2 ^ x % 2) ^ y % 2)
                {
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[7], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[2], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[7], cubeIndices[5], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[2], cubeIndices[0], cubeIndices[5]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[6], cubeIndices[7], cubeIndices[5]));
                }
                else
                {
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[3], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[6], cubeIndices[2], cubeIndices[1]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[6], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[3], cubeIndices[1], cubeIndices[4]));
                }
            }
        }
    }

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                uvCoords[x + dim[0] * (y + dim[1] * z)] = Vec2f(static_cast<float>(x) / dim[0], static_cast<float>(z) / dim[2]) * 3.0;
            }
        }
    }

    // Ensure correct windings
    for (int i = 0; i < indices.size(); i++)
    {
        if (tetVolume(vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]]) < 0.0)
        {
            std::swap(indices[i][0], indices[i][2]);
        }
    }

    tissueMesh->initialize(verticesPtr, indicesPtr);
    tissueMesh->setVertexTCoords("uvs", uvCoordsPtr);

    m_objectTetMesh = tissueMesh;
}

void
InflatableObject::setSphereTexCoords(const double uvScale)
{
    Vec3d min, max;
    m_objectSurfMesh->computeBoundingBox(min, max);
    const Vec3d size   = max - min;
    const Vec3d center = (max + min) * 0.5;

    const double radius = (size * 0.5).norm();

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(m_objectSurfMesh->getNumVertices());
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < m_objectSurfMesh->getNumVertices(); i++)
    {
        Vec3d vertex = m_objectSurfMesh->getVertexPosition(i) - center;

        // Compute phi and theta on the sphere
        const double theta = asin(vertex[0] / radius);
        const double phi   = atan2(vertex[1], vertex[2]);
        uvCoords[i] = Vec2f(phi / (PI * 2.0) + 0.5, theta / (PI * 2.0) + 0.5) * uvScale;
    }
    m_objectSurfMesh->setVertexTCoords("tcoords", uvCoordsPtr);
}

void
InflatableObject::findAffectedConstraint(const Vec3d& toolTip, const double radius)
{
    m_constraintIDandWeight.clear();

    Vec3d min, max;
    m_objectTetMesh->computeBoundingBox(min, max);
    if (!CollisionUtils::testAABBToAABB(toolTip[0], toolTip[0], toolTip[1], toolTip[1], toolTip[2], toolTip[2],
        min[0], max[0], min[1], max[1], min[2], max[2]))
    {
        return;
    }

    const auto& vertices = m_objectTetMesh->getVertexPositions();

    std::vector<int>    constraintIDs;
    std::vector<double> weights;
    int                 counter     = 0;
    double              minDistance = LONG_MAX;
    for (auto& c : m_constraintContainer->getConstraints())
    {
        auto& ids = c->getVertexIds();

        Vec3d center(0.0, 0.0, 0.0);
        for (auto i : ids)
        {
            center += (*vertices)[i];
        }

        double distance = (center / ids.size() - toolTip).norm();

        if (distance < radius)
        {
            constraintIDs.push_back(counter);
            weights.push_back(computeGaussianWeight(distance));
        }

        if (distance < minDistance)
        {
            minDistance = distance;
        }

        counter++;
    }

    if (minDistance > 0.5)
    {
        return;
    }
    else
    {
        for (int i = 0; i < constraintIDs.size(); i++)
        {
            m_constraintIDandWeight.push_back(std::make_pair(constraintIDs[i], weights[i]));
        }

        m_affectedAreaUpdated = true;
    }
}

void
InflatableObject::inject(const Vec3d& toolTip, const double radius, double dx)
{
    if (!m_affectedAreaUpdated)
    {
        findAffectedConstraint(toolTip, radius);
    }

    double de = 0.0;
    if (m_inflationType == InflationType::Exponential)
    {
        de = std::exp(dx);
    }
    else if (m_inflationType == InflationType::Linear)
    {
        de = dx;
    }

    for (auto& id : m_constraintIDandWeight)
    {
        const double dv = id.second * de;

        auto& c = (m_constraintContainer->getConstraints())[id.first];
        if (c->getType() == PbdConstraint::Type::Volume)
        {
            const auto& constraint = std::dynamic_pointer_cast<PbdInflatableVolumeConstraint>(c);
            constraint->setRestVolume(dv + constraint->getRestVolume());
        }
        else if (c->getType() == PbdConstraint::Type::Distance)
        {
            // If a tets volume scales by X then each of it sides grow by ?
            /* const auto& constraint = std::dynamic_pointer_cast<PbdInflatableDistanceConstraint>(c);
             constraint->setRestLength(dv + constraint->getRestLength());*/
        }
    }
}

void
InflatableObject::switchInflationType()
{
    if (m_inflationType == InflationType::Linear)
    {
        m_inflationType = InflationType::Exponential;
        std::cout << "Inflation Type: Exponential." << std::endl;
    }
    else if (m_inflationType == InflationType::Exponential)
    {
        m_inflationType = InflationType::Linear;
        std::cout << "Inflation Type: Linear." << std::endl;
    }
}

void
InflatableObject::setUpdateAffectedConstraint()
{
    m_affectedAreaUpdated = false;
    m_inflationRatio      = 1.0;
}

void
InflatableObject::reset()
{
    PbdObject::reset();

    m_inflationRatio = 1.0;

    for (auto& c : m_constraintContainer->getConstraints())
    {
        if (c->getType() == PbdConstraint::Type::Volume)
        {
            const auto& constraint = std::dynamic_pointer_cast<PbdInflatableVolumeConstraint>(c);
            constraint->resetRestVolume();
        }
        else if (c->getType() == PbdConstraint::Type::Distance)
        {
            const auto& constraint = std::dynamic_pointer_cast<PbdInflatableDistanceConstraint>(c);
            constraint->resetRestLength();
        }
    }
}
