///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//	  http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#include "imstkSimulationManager.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkPBDCollisionHandling.h"
#include "imstkTetraTriangleMap.h"

using namespace imstk;

// Creates a non-manifold top part of a staircase
static std::unique_ptr<SurfaceMesh> buildStairs(int nSteps, double width, double height, double depth)
{
	// Build stair geometry
	const double halfWidth = width * 0.5;
	const double halfHeight = height * 0.5;
	const double halfDepth = depth * 0.5;
	const double dz = depth / static_cast<double>(nSteps);
	const double dy = height / static_cast<double>(nSteps);

	// Create vertices
	StdVectorOfVec3d vertList;
	// 4 verts per step, 2 back, then 2 bottom
	vertList.reserve(nSteps * 4 + 4);
	for (size_t i = 0; i < nSteps; i++)
	{
		const double z = static_cast<double>(dz * i) - halfDepth;
		const double y1 = static_cast<double>(dy * i) - halfHeight;
		vertList.push_back(Vec3d(-halfWidth, y1, z));
		vertList.push_back(Vec3d(halfWidth, y1, z));

		const double y2 = static_cast<double>(dy * (i + 1)) - halfHeight;
		vertList.push_back(Vec3d(-halfWidth, y2, z));
		vertList.push_back(Vec3d(halfWidth, y2, z));
	}
	{
		const double z = static_cast<double>(dz * nSteps) - halfDepth;
		const double yTop = static_cast<double>(dy * nSteps) - halfHeight;
		vertList.push_back(Vec3d(-halfWidth, yTop, z));
		vertList.push_back(Vec3d(halfWidth, yTop, z));

		const double yBot = -halfHeight;
		vertList.push_back(Vec3d(-halfWidth, yBot, z));
		vertList.push_back(Vec3d(halfWidth, yBot, z));
	}

	// Create cells
	std::vector<SurfaceMesh::TriangleArray> triangles;
	// Create sides and tops of steps
	for (std::size_t i = 0; i < nSteps; ++i)
	{
		// Stair front side
		triangles.push_back({ { i * 4 + 3, i * 4 + 1, i * 4 } });
		triangles.push_back({ { i * 4 + 2, i * 4 + 3, i * 4 } });
		// Stair top
		triangles.push_back({ { (i + 1) * 4, i * 4 + 3, i * 4 + 2 } });
		triangles.push_back({ { (i + 1) * 4, (i + 1) * 4 + 1, i * 4 + 3 } });
	}

	std::unique_ptr<SurfaceMesh> stairMesh = std::make_unique<SurfaceMesh>();
	stairMesh->initialize(vertList, triangles);
	return stairMesh;
}

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamic on a more complex mesh
///
int
main()
{
	auto simManager = std::make_shared<SimulationManager>();
	auto scene = simManager->createNewScene("PbdCollision");
	scene->getCamera()->setPosition(0.0, 0.0, -30.0);
	scene->getCamera()->setFocalPoint(0.0, 0.0, 0.0);

	auto deformableObj = std::make_shared<PbdObject>("DeformableObj");
	{
		// Read in the dragon mesh
		auto highResSurfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj"));
		auto coarseTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg"));
		highResSurfMesh->translate(Vec3d(0.0f, 10.0f, 0.0f), Geometry::TransformType::ApplyToData);
		coarseTetMesh->translate(Vec3d(0.0f, 10.0f, 0.0f), Geometry::TransformType::ApplyToData);
		auto coarseSurfMesh = std::make_shared<SurfaceMesh>();
		coarseTetMesh->extractSurfaceMesh(coarseSurfMesh, true);

		// Setup parameters
		auto pbdParams = std::make_shared<PBDModelConfig>();
		pbdParams->m_YoungModulus = 1000.0;
		pbdParams->m_PoissonRatio = 0.3;
		pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet,
			PbdFEMConstraint::MaterialType::StVK);
		pbdParams->m_uniformMassValue = 1.0;
		pbdParams->m_gravity = Vec3d(0, -10.0, 0);
		pbdParams->m_DefaultDt = 0.01;
		pbdParams->m_maxIter = 5;
		pbdParams->m_proximity = 0.3;
		pbdParams->m_contactStiffness = 0.1;

		// Setup Model
		auto pbdModel = std::make_shared<PbdModel>();
		pbdModel->setModelGeometry(coarseTetMesh);
		pbdModel->configure(pbdParams);

		// Setup VisualModel
		auto material = std::make_shared<RenderMaterial>();
		material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
		auto surfMeshModel = std::make_shared<VisualModel>(highResSurfMesh);
		surfMeshModel->setRenderMaterial(material);

		// Setup Object
		deformableObj->addVisualModel(surfMeshModel);
		deformableObj->setCollidingGeometry(coarseSurfMesh);
		deformableObj->setPhysicsGeometry(coarseTetMesh);
		deformableObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(coarseTetMesh, coarseSurfMesh));
		deformableObj->setPhysicsToVisualMap(std::make_shared<TetraTriangleMap>(coarseTetMesh, highResSurfMesh));
		deformableObj->setDynamicalModel(pbdModel);

		// Add to scene
		scene->addSceneObject(deformableObj);
	}

	auto stairObj = std::make_shared<PbdObject>("Floor");
	{
		std::shared_ptr<SurfaceMesh> stairMesh(std::move(buildStairs(15, 20.0, 10.0, 20.0)));

		// Setup parameters
		auto pbdParams2 = std::make_shared<PBDModelConfig>();
		pbdParams2->m_uniformMassValue = 0.0;
		pbdParams2->m_proximity = 0.1;
		pbdParams2->m_contactStiffness = 1.0;
		pbdParams2->m_maxIter = 0;

		// Setup model
		auto pbdModel2 = std::make_shared<PbdModel>();
		pbdModel2->setModelGeometry(stairMesh);
		pbdModel2->configure(pbdParams2);

		// Setup VisualModel
		auto stairMaterial = std::make_shared<RenderMaterial>();
		stairMaterial->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
		auto stairMeshModel = std::make_shared<VisualModel>(stairMesh);
		stairMeshModel->setRenderMaterial(stairMaterial);

		stairObj->addVisualModel(stairMeshModel);
		stairObj->setDynamicalModel(pbdModel2);
		stairObj->setCollidingGeometry(stairMesh);
		stairObj->setVisualGeometry(stairMesh);
		stairObj->setPhysicsGeometry(stairMesh);

		scene->addSceneObject(stairObj);
	}

	// Collision
	scene->getCollisionGraph()->addInteractionPair(deformableObj, stairObj,
		CollisionDetection::Type::MeshToMeshBruteForce,
		CollisionHandling::Type::PBD,
		CollisionHandling::Type::None);

	// Light
	auto light = std::make_shared<DirectionalLight>("light");
	light->setFocalPoint(Vec3d(5, -8, 5));
	scene->addLight(light);

	simManager->setActiveScene(scene);
	simManager->start(SimulationStatus::paused);

    return 0;
}
