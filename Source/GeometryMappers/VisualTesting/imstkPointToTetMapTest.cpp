/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollider.h"
#include "imstkGeometryUtilities.h"
#include "imstkPbdSystem.h"
#include "imstkPbdSystemConfig.h"
#include "imstkPointToTetMap.h"
#include "imstkPointwiseMap.h"
#include "imstkScene.h"
#include "imstkSceneUtils.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

///
/// \brief Tests that the PointToTetMap gets setup correctly when used in
/// the scene with visual and collision geometry maps
///
TEST_F(VisualTest, PointToTetMapTest)
{
    m_scene = std::make_shared<Scene>("testScene");
    m_scene->getActiveCamera()->setPosition(0.0, 0.4, -0.7);
    m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    // Setup a physics geometry
    std::shared_ptr<Entity> tissueObj;
    {
        // Setup the Geometry
        std::shared_ptr<TetrahedralMesh> tetMeshFine =
            GeometryUtils::toTetGrid(Vec3d::Zero(), Vec3d(0.1, 0.1, 0.1), Vec3i(10, 10, 10));
        std::shared_ptr<SurfaceMesh> tetMeshFine_sf = tetMeshFine->extractSurfaceMesh();

        const Vec3i                      coarseDim(5, 5, 5);
        std::shared_ptr<TetrahedralMesh> tetMeshCoarse =
            GeometryUtils::toTetGrid(Vec3d::Zero(), Vec3d(0.1, 0.1, 0.1), coarseDim);
        std::shared_ptr<SurfaceMesh> tetMeshCoarse_sf = tetMeshCoarse->extractSurfaceMesh();

        // Setup the Model
        auto pbdSystem = std::make_shared<PbdSystem>();
        pbdSystem->getConfig()->m_doPartitioning = false;
        pbdSystem->getConfig()->m_gravity    = Vec3d(0.0, -9.8, 0.0);
        pbdSystem->getConfig()->m_iterations = 8;
        pbdSystem->getConfig()->m_dt = 0.001;
        pbdSystem->getConfig()->m_linearDampingCoeff = 0.025;

        // Setup the Object
        tissueObj = imstk::SceneUtils::makePbdEntity("tissueObj", tetMeshFine_sf, tetMeshCoarse_sf, tetMeshCoarse, pbdSystem);
        auto tissueMethod = tissueObj->getComponent<PbdMethod>();
        tissueMethod->setPhysicsToVisualMap(std::make_shared<PointToTetMap>(tetMeshCoarse, tetMeshFine_sf));
        tissueMethod->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMeshCoarse, tetMeshCoarse_sf));
        tissueMethod->getPbdBody()->uniformMassValue = 0.01;

        pbdSystem->getConfig()->m_secParams->m_YoungModulus = 1000.0;
        pbdSystem->getConfig()->m_secParams->m_PoissonRatio = 0.45; // 0.48 for tissue
        pbdSystem->getConfig()->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::StVK,
            tissueMethod->getPbdBody()->bodyHandle);

        // Fix the borders
        for (int z = 0; z < coarseDim[2]; z++)
        {
            for (int y = 0; y < coarseDim[1]; y++)
            {
                for (int x = 0; x < coarseDim[0]; x++)
                {
                    if (x == 0 || z == 0 || x == coarseDim[0] - 1 || z == coarseDim[2] - 1)
                    {
                        tissueMethod->getPbdBody()->fixedNodeIds.push_back(x + coarseDim[0] * (y + coarseDim[1] * z));
                    }
                }
            }
        }
    }
    m_scene->addSceneObject(tissueObj);

    runFor(2.0, 0.001);
}