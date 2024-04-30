/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include <gtest/gtest.h>

#include <imstkPbdObject.h>
#include <imstkTetrahedralMesh.h>
#include <imstkGeometryUtilities.h>
#include <imstkSurfaceMesh.h>
#include <imstkPointwiseMap.h>
#include <imstkPbdModel.h>
#include <imstkPbdModelConfig.h>
#include <imstkPbdFemConstraint.h>
#include <imstkPbdObjectCellRemoval.h>
#include <imstkPbdDistanceConstraint.h>
#include <imstkMeshIO.h>
#include <imstkPointwiseMap.h>

using namespace imstk;

namespace
{
std::shared_ptr<PbdObject>
build(const std::string&               name,
      std::shared_ptr<PbdModel>        model,
      std::shared_ptr<TetrahedralMesh> tetMesh)
{
    auto visualMesh    = tetMesh->extractSurfaceMesh();
    auto collisionMesh = tetMesh->extractSurfaceMesh();

    auto pbdObject = std::make_shared<PbdObject>(name);
    // Setup the Object
    pbdObject->setPhysicsGeometry(tetMesh);
    pbdObject->setVisualGeometry(visualMesh);
    pbdObject->setCollidingGeometry(collisionMesh);

    {
        auto map = std::make_shared<PointwiseMap>();
        map->setParentGeometry(tetMesh);
        map->setChildGeometry(collisionMesh);
        pbdObject->setPhysicsToCollidingMap(map);
    }

    {
        auto map = std::make_shared<PointwiseMap>();
        map->setParentGeometry(tetMesh);
        map->setChildGeometry(visualMesh);
        pbdObject->setPhysicsToVisualMap(map);
    }

    pbdObject->setDynamicalModel(model);
    pbdObject->getPbdBody()->uniformMassValue = 0.01;

    model->getConfig()->m_femParams->m_YoungModulus = 1000.0;
    model->getConfig()->m_femParams->m_PoissonRatio = 0.45;                 // 0.48 for tissue
    model->getConfig()->enableFemConstraint(PbdFemConstraint::MaterialType::StVK,
                        pbdObject->getPbdBody()->bodyHandle);
    pbdObject->getPbdBody()->bodyGravity = false;

    return pbdObject;
}

static std::shared_ptr<PbdObject>
makeCubeTetObj(const std::string& name,
               std::shared_ptr<PbdModel> model,
               const Vec3d& size = Vec3d(1, 1, 1), const Vec3i& dim = Vec3i(2, 2, 2), const Vec3d& center = Vec3d::Zero(),
               const Quatd& orientation = Quatd::Identity())
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh =
        GeometryUtils::toTetGrid(center, size, dim, orientation);

    for (int i = 0; i < tetMesh->getNumTetrahedra(); i++)
    {
        std::swap((*tetMesh->getCells())[i][2], (*tetMesh->getCells())[i][3]);
    }

    auto tissueObj = build(name, model, tetMesh);
    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || z == 0 || x == dim[0] - 1 || z == dim[2] - 1)
                {
                    tissueObj->getPbdBody()->fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    return tissueObj;
}

int
count(const imstk::VecDataArray<int, 4> v, Vec4i item = Vec4i::Zero())
{
    return std::count(v.begin(), v.end(), item);
}

int
count(const imstk::VecDataArray<int, 3> v, Vec3i item = Vec3i::Zero())
{
    return std::count(v.begin(), v.end(), item);
}
} // namespace

struct imstkCellRemovalTest : testing::Test
{
    void SetUp() override
    {
        pbdModel = std::make_shared<PbdModel>();
        pbdModel->getConfig()->m_doPartitioning = false;
        pbdModel->getConfig()->m_dt = 0.001;
        pbdModel->getConfig()->m_iterations = 5;
        pbdModel->getConfig()->m_linearDampingCoeff = 0.025;

        pbdObject = makeCubeTetObj("Test", pbdModel);
        pbdObject->initialize();

        volumeMesh    = std::dynamic_pointer_cast<TetrahedralMesh>(pbdObject->getPhysicsGeometry());
        visualMesh    = std::dynamic_pointer_cast<SurfaceMesh>(pbdObject->getVisualGeometry());
        collisionMesh = std::dynamic_pointer_cast<SurfaceMesh>(pbdObject->getCollidingGeometry());
    }

    std::shared_ptr<PbdModel> pbdModel;
    std::shared_ptr<PbdObject> pbdObject;

    std::shared_ptr<TetrahedralMesh> volumeMesh;
    std::shared_ptr<SurfaceMesh> visualMesh;
    std::shared_ptr<SurfaceMesh> collisionMesh;
};

TEST_F(imstkCellRemovalTest, Consistency)
{
    pbdModel->initialize();
    // Just verifies some basic assumptions
    ASSERT_NE(nullptr, volumeMesh);
    ASSERT_NE(nullptr, visualMesh);
    ASSERT_NE(nullptr, collisionMesh);
    ASSERT_NE(visualMesh, collisionMesh);

    EXPECT_EQ(5, volumeMesh->getNumTetrahedra());
    EXPECT_EQ(5, pbdModel->getConstraints()->getConstraints().size());
    EXPECT_EQ(12, visualMesh->getNumTriangles());
    EXPECT_EQ(12, collisionMesh->getNumTriangles());

    EXPECT_EQ(8, volumeMesh->getNumVertices());
    EXPECT_EQ(8, visualMesh->getNumVertices());
    EXPECT_EQ(8, collisionMesh->getNumVertices());
}

TEST_F(imstkCellRemovalTest, RemovalWithoutUpdate)
{
    auto remover = std::make_shared<PbdObjectCellRemoval>(pbdObject, PbdObjectCellRemoval::OtherMeshUpdateType::None);

    remover->initialize();
    pbdModel->initialize();

    // Remover added the phantom vertex
    EXPECT_EQ(9, volumeMesh->getNumVertices());
    EXPECT_EQ(8, visualMesh->getNumVertices());
    EXPECT_EQ(8, collisionMesh->getNumVertices());

    EXPECT_EQ(5, pbdModel->getConstraints()->getConstraints().size());

    remover->removeCellOnApply(0);
    remover->apply();

    // Tetrahedron doesn't _really_ get remove but set to the [0,0,0]
    EXPECT_EQ(5, volumeMesh->getNumTetrahedra());
    EXPECT_EQ(Vec4i(0, 0, 0, 0), volumeMesh->getTetrahedraIndices()->at(0));

    EXPECT_EQ(4, pbdModel->getConstraints()->getConstraints().size());

    for (int i = 0; i < volumeMesh->getNumTetrahedra(); ++i)
    {
        remover->removeCellOnApply(i);
    }

    remover->apply();

    for (int i = 0; i < volumeMesh->getNumTetrahedra(); ++i)
    {
        EXPECT_EQ(Vec4i(0, 0, 0, 0), volumeMesh->getTetrahedraIndices()->at(i));
    }

    EXPECT_EQ(0, pbdModel->getConstraints()->getConstraints().size());
}

TEST_F(imstkCellRemovalTest, RemovalWithUpdate)
{
    auto remover = std::make_shared<PbdObjectCellRemoval>(pbdObject, PbdObjectCellRemoval::OtherMeshUpdateType::CollisionAndVisualReused);

    remover->initialize();
    pbdModel->initialize();

    // Remover added the phantom vertex
    EXPECT_EQ(9, volumeMesh->getNumVertices());
    EXPECT_EQ(9, visualMesh->getNumVertices());
    EXPECT_EQ(9, collisionMesh->getNumVertices());

    remover->removeCellOnApply(0);
    remover->apply();

    // Tetrahedron doesn't _really_ get remove but set to the [0,0,0]
    EXPECT_EQ(5, volumeMesh->getNumTetrahedra());
    EXPECT_EQ(Vec4i(0, 0, 0, 0), volumeMesh->getTetrahedraIndices()->at(0));

    // Vertices shouldn't change it's only corners
    EXPECT_EQ(9, visualMesh->getNumVertices());
    EXPECT_EQ(9, collisionMesh->getNumVertices());

    for (int i = 0; i < volumeMesh->getNumTetrahedra(); ++i)
    {
        remover->removeCellOnApply(i);
    }

    remover->apply();

    EXPECT_EQ(volumeMesh->getNumTetrahedra(),
            count(*volumeMesh->getTetrahedraIndices(), Vec4i::Zero()));

    // Vertices shouldn't change it's only corners
    EXPECT_EQ(9, visualMesh->getNumVertices());
    EXPECT_EQ(9, collisionMesh->getNumVertices());

    // All triangles should be set to 0
    EXPECT_EQ(visualMesh->getNumTriangles(),
            count(*visualMesh->getTriangleIndices(), Vec3i::Zero()));
    EXPECT_EQ(collisionMesh->getNumTriangles(),
            count(*collisionMesh->getTriangleIndices(), Vec3i::Zero()));
}

TEST_F(imstkCellRemovalTest, TwoBodyConstraints)
{
    // Check in Unity if the ordering between generating the constraints
    // and the remover being added/initialized is correct

    auto otherPbdObject = makeCubeTetObj("Test2", pbdModel);
    otherPbdObject->initialize();

    auto remover = std::make_shared<PbdObjectCellRemoval>(pbdObject, PbdObjectCellRemoval::OtherMeshUpdateType::CollisionAndVisualReused);
    remover->initialize();

    pbdModel->initialize();
    EXPECT_EQ(10, pbdModel->getConstraints()->getConstraints().size());

    for (int i = 1; i < volumeMesh->getNumVertices(); ++i)
    {
        auto          c = std::make_shared<PbdDistanceConstraint>();
        PbdParticleId p0(pbdObject->getPbdBody()->bodyHandle, i);
        PbdParticleId p1(otherPbdObject->getPbdBody()->bodyHandle, i - 1);
        c->initConstraint(0, p0, p1);
        pbdModel->getConstraints()->addConstraint(c);
    }

    // Tetrahedra * 2 + VertexConstraints
    EXPECT_EQ(2 * 5 + 8, pbdModel->getConstraints()->getConstraints().size());

    // Remover added the phantom vertex
    EXPECT_EQ(9, volumeMesh->getNumVertices());
    EXPECT_EQ(9, visualMesh->getNumVertices());
    EXPECT_EQ(9, collisionMesh->getNumVertices());

    remover->removeCellOnApply(0);
    remover->apply();

    // We're over-removing body/body constraints all constraints on the
    // removed tetrahedron are removed, even though some of the vertices
    // are still "alive"
    EXPECT_EQ(5 + 4 + 4, pbdModel->getConstraints()->getConstraints().size());

    for (int i = 0; i < volumeMesh->getNumTetrahedra(); ++i)
    {
        remover->removeCellOnApply(i);
    }

    remover->apply();
    EXPECT_EQ(5, pbdModel->getConstraints()->getConstraints().size());
}

std::vector<std::pair<int, int>> verifyMap(std::shared_ptr<TetrahedralMesh> volumeMesh,
	std::shared_ptr<SurfaceMesh>surfaceMesh, std::shared_ptr<PointwiseMap> map)
{
	std::vector<std::pair<int, int>> invalidPoints;
	// Gather all the actual points in the tetrahedron
	std::unordered_set<int> tetPoints;
	for (const auto& tet : *volumeMesh->getTetrahedraIndices())
	{
		tetPoints.insert(tet[0]);
		tetPoints.insert(tet[1]);
		tetPoints.insert(tet[2]);
		tetPoints.insert(tet[3]);
	}


	std::unordered_set<int> surfacePoints;
	for (const auto& tri : *surfaceMesh->getTriangleIndices())
	{
		surfacePoints.insert(tri[0]);
		surfacePoints.insert(tri[1]);
		surfacePoints.insert(tri[2]);
	}

	auto m = map->getMap();
	for (int i : surfacePoints)
	{
		if (tetPoints.find(m[i]) == tetPoints.end())
		{
			invalidPoints.push_back({ i, m[i] });
		}
	}
	return invalidPoints;
}

TEST_F(imstkCellRemovalTest, validateMaps)
{
	auto remover = std::make_shared<PbdObjectCellRemoval>(pbdObject, PbdObjectCellRemoval::OtherMeshUpdateType::CollisionAndVisualReused);

	remover->initialize();
	pbdModel->initialize();


    auto pointwiseMap = std::dynamic_pointer_cast<PointwiseMap>(pbdObject->getPhysicsToCollidingMap());

    ASSERT_NE(nullptr, pointwiseMap);
    EXPECT_TRUE(verifyMap(volumeMesh, visualMesh, pointwiseMap).size() == 0);

	remover->removeCellOnApply(0);
	remover->apply();

    EXPECT_TRUE(verifyMap(volumeMesh, visualMesh, pointwiseMap).size() == 0);

}