/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "InflatableObject.h"
#include "imstkCollisionUtils.h"
#include "imstkImageData.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkPbdInflatableDistanceConstraint.h"
#include "imstkPbdInflatableVolumeConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"
#include "imstkGeometryUtilities.h"

InflatableObject::InflatableObject(const std::string& name, const Vec3d& tissueSize, const Vec3i& tissueDim, const Vec3d& tissueCenter) : PbdObject(name)
{
    // Setup the Geometry
    m_objectTetMesh  = GeometryUtils::toTetGrid(tissueCenter, tissueSize, tissueDim);
    m_objectSurfMesh = m_objectTetMesh->extractSurfaceMesh();
    setSphereTexCoords(4.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.1;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 2;
    pbdParams->m_viscousDampingCoeff = 0.05;

    // Add custom constraint generation functors
    auto distanceFunctor = std::make_shared<PbdInflatableDistanceConstraintFunctor>();
    distanceFunctor->setStiffness(0.95);
    auto volumeFunctor = std::make_shared<PbdInflatableVolumeConstraintFunctor>();
    volumeFunctor->setStiffness(0.9);

    pbdParams->addPbdConstraintFunctor(distanceFunctor);
    pbdParams->addPbdConstraintFunctor(volumeFunctor);

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

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(m_objectSurfMesh);
    visualModel->setRenderMaterial(material);
    addVisualModel(visualModel);

    // Setup the Object
    setPhysicsGeometry(m_objectTetMesh);
    setCollidingGeometry(m_objectSurfMesh);
    setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(m_objectTetMesh, m_objectSurfMesh));
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
    if (!CollisionUtils::testAabbToAabb(toolTip[0], toolTip[0], toolTip[1], toolTip[1], toolTip[2], toolTip[2],
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
        for (size_t i = 0; i < constraintIDs.size(); i++)
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
        if (auto volConstraint = std::dynamic_pointer_cast<PbdInflatableVolumeConstraint>(c))
        {
            volConstraint->setRestVolume(dv + volConstraint->getRestVolume());
            volConstraint->setStiffness(1.0);
        }
        else if (auto distConstraint = std::dynamic_pointer_cast<PbdInflatableDistanceConstraint>(c))
        {
            distConstraint->setRestLength(0.00001 * cbrt(dv) + distConstraint->getRestLength());
            distConstraint->setStiffness(0.1);
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
        if (auto volConstraint = std::dynamic_pointer_cast<PbdInflatableVolumeConstraint>(c))
        {
            volConstraint->resetRestVolume();
        }
        else if (auto distConstraint = std::dynamic_pointer_cast<PbdInflatableDistanceConstraint>(c))
        {
            distConstraint->resetRestLength();
        }
    }
}
