#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkMath.h"
#include "imstkSimulationManager.h"
#include "imstkSceneObject.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkLight.h"
#include "imstkCamera.h"

// Geometry
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshReader.h"

// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

#include "g3log/g3log.hpp"

void testViewer();
void testReadMesh();
void testAnalyticalGeometry();
void testScenesManagement();
void testIsometricMap();
void testTetraTriangleMap();
void testOneToOneNodalMap();
void testExtractSurfaceMesh();
void testSurfaceMeshOptimizer();

int main()
{
    std::cout << "****************\n"
              << "Starting Sandbox\n"
              << "****************\n";

    //testViewer();
    testReadMesh();
    //testAnalyticalGeometry();
    //testScenesManagement();
    //testIsometricMap();
    //testTetraTriangleMap();
    //testExtractSurfaceMesh();
    //testOneToOneNodalMap();
    //testSurfaceMeshOptimizer();

    return 0;
}

void testReadMesh()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto scene = sdk->createNewScene("SceneTestMesh");
    scene->setLoopDelay(1000);

    // Read surface mesh
    /*
    auto objMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/asianDragon/asianDragon.obj");
    auto plyMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.ply");
    auto stlMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.stl");
    auto vtkMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.vtk");
    auto vtpMesh = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Cube/models/cube.vtp");
    */

    // Read volumetricMesh
    auto mesh = imstk::MeshReader::read("nidus10KTet.vtk");
    auto volumeMesh = std::dynamic_pointer_cast<imstk::VolumetricMesh>(mesh);

    auto surfaceMesh = std::make_shared<imstk::SurfaceMesh>();
    volumeMesh->extractSurfaceMesh(surfaceMesh);

    // Create object and add to scene
    auto object = std::make_shared<imstk::VisualObject>("meshObject");
    object->setVisualGeometry(surfaceMesh);
    scene->addSceneObject(object);

    // Run
    sdk->setCurrentScene("SceneTestMesh");
    sdk->startSimulation(true);
}

void testViewer()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto sceneTest = sdk->createNewScene("SceneTest");
    sceneTest->setLoopDelay(1000);

    // Plane
    auto planeGeom = std::make_shared<imstk::Plane>();
    planeGeom->scale(10);
    auto planeObj = std::make_shared<imstk::VisualObject>("VisualPlane");
    planeObj->setVisualGeometry(planeGeom);

    // Cube
    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    cubeGeom->rotate(imstk::UP_VECTOR, imstk::PI_4);
    cubeGeom->rotate(imstk::RIGHT_VECTOR, imstk::PI_4);
    cubeGeom->translate(1.0, - 1.0, 0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    sphereGeom->translate(0, 2, 0);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Light (white)
    auto whiteLight = std::make_shared<imstk::Light>("whiteLight");
    whiteLight->setPosition(imstk::Vec3d(5, 8, 5));
    whiteLight->setPositional();

    // Light (red)
    auto colorLight = std::make_shared<imstk::Light>("colorLight");
    colorLight->setPosition(imstk::Vec3d(4, -3, 1));
    colorLight->setFocalPoint(imstk::Vec3d(0, 0, 0));
    colorLight->setColor(imstk::Color::Red);
    colorLight->setPositional();
    colorLight->setSpotAngle(15);

    // Add in scene
    sceneTest->addSceneObject(planeObj);
    sceneTest->addSceneObject(cubeObj);
    sceneTest->addSceneObject(sphereObj);
    sceneTest->addLight(whiteLight);
    sceneTest->addLight(colorLight);

    // Update Camera
    auto cam1 = sceneTest->getCamera();
    cam1->setPosition(imstk::Vec3d(-5.5, 2.5, 32));
    cam1->setFocalPoint(imstk::Vec3d(1, 1, 0));

    // Run
    sdk->setCurrentScene("SceneTest");
    sdk->startSimulation(true);
}

void testAnalyticalGeometry()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Plane
    LOG(INFO) << "-- Plane : Init";
    auto pos   = imstk::Vec3d(5, 2, 5);
    auto norm  = imstk::Vec3d(0, 1, 1);
    auto width = 10;
    LOG(INFO) << "p = " << pos;
    LOG(INFO) << "n = " << norm;
    LOG(INFO) << "w = " << width;

    LOG(INFO) << "-- Plane : Create";
    auto plane = std::make_shared<imstk::Plane>(pos, norm, width);
    LOG(INFO) << "p = " << plane->getPosition();
    LOG(INFO) << "n = " << plane->getNormal();
    LOG(INFO) << "w = " << plane->getWidth();

    LOG(INFO) << "-- Plane : Set Position";
    plane->setPosition(imstk::Vec3d(1, 1, 1));
    LOG(INFO) << "p = " << plane->getPosition();

    LOG(INFO) << "-- Plane : Translate";
    plane->translate(imstk::Vec3d(2, 1, -3));
    LOG(INFO) << "p = " << plane->getPosition();

    LOG(INFO) << "-- Plane : Set Normal";
    plane->setNormal(imstk::FORWARD_VECTOR);
    LOG(INFO) << "n = " << plane->getNormal();

    LOG(INFO) << "-- Plane : Rotate";
    plane->rotate(imstk::UP_VECTOR, imstk::PI_2);
    LOG(INFO) << "n = " << plane->getNormal();
}

void testScenesManagement()
{
    // THIS TESTS NEEDS TO DISABLE STANDALONE VIEWER RENDERING

    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Scenes
    LOG(INFO) << "-- Test add scenes";
    auto scene1 = std::make_shared<imstk::Scene>("scene1");
    scene1->setLoopDelay(500);
    sdk->addScene(scene1);

    sdk->createNewScene("scene2");
    auto scene2 = sdk->getScene("scene2");
    scene2->setLoopDelay(500);

    auto scene3 = sdk->createNewScene();
    sdk->removeScene("Scene_3");

    // switch
    LOG(INFO) << "-- Test scene switch";
    int delay = 5;
    sdk->setCurrentScene("scene1");
    sdk->startSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setCurrentScene("scene2", false);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->setCurrentScene("scene1", true);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->endSimulation();

    // pause/run
    LOG(INFO) << "-- Test simulation pause/run";
    sdk->setCurrentScene("scene2");
    sdk->startSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->runSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->pauseSimulation();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    sdk->endSimulation();

    // Quit
    while (sdk->getStatus() != imstk::SimulationStatus::INACTIVE) {}
}

void testIsometricMap()
{
    // SDK and Scene
    auto sdk = std::make_shared<imstk::SimulationManager>();
    auto geometryMapTest = sdk->createNewScene("geometryMapTest");
    geometryMapTest->setLoopDelay(1000);

    // Cube
    auto cubeGeom = std::make_shared<imstk::Cube>();
    cubeGeom->scale(0.5);
    auto cubeObj = std::make_shared<imstk::VisualObject>("VisualCube");
    cubeObj->setVisualGeometry(cubeGeom);

    // Sphere
    auto sphereGeom = std::make_shared<imstk::Sphere>();
    sphereGeom->scale(0.3);
    auto sphereObj = std::make_shared<imstk::VisualObject>("VisualSphere");
    sphereObj->setVisualGeometry(sphereGeom);

    // Add objects in Scene
    geometryMapTest->addSceneObject(cubeObj);
    geometryMapTest->addSceneObject(sphereObj);

    // Isometric Map
    auto transform = imstk::RigidTransform3d::Identity();
    transform.translate(imstk::Vec3d(0.0, 1.0, 0.0));
    transform.rotate(Eigen::AngleAxisd(imstk::PI_4, imstk::Vec3d(0, 1.0, 0)));

    auto rigidMap = std::make_shared<imstk::IsometricMap>();
    rigidMap->setMaster(sphereObj->getVisualGeometry());
    rigidMap->setSlave(cubeObj->getVisualGeometry());
    rigidMap->setTransform(transform);

    // Test map
    LOG(INFO) << cubeGeom->getPosition();

    rigidMap->apply();
    LOG(INFO) << cubeGeom->getPosition();

    sphereGeom->setPosition(1.0, 0.0, 1.0);
    rigidMap->apply();
    LOG(INFO) << cubeGeom->getPosition();

    // Start simulation
    sdk->setCurrentScene("geometryMapTest");
    sdk->startSimulation(imstk::Renderer::Mode::DEBUG);
}

void testTetraTriangleMap()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // Tetrahedral mesh
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(1.0, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1.0, 0));
    vertList.push_back(imstk::Vec3d(0, 0, 1.0));
    tetMesh->setInitialVerticesPositions(vertList);
    tetMesh->setVerticesPositions(vertList);

    std::vector<imstk::TetrahedralMesh::TetraArray> tetConnectivity;
    imstk::TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    tetConnectivity.push_back(tet1);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // Triangular mesh
    auto triMesh = std::make_shared<imstk::SurfaceMesh>();
    std::vector<imstk::Vec3d> SurfVertList;
    SurfVertList.push_back(imstk::Vec3d(0, 0, 1));// coincides with one vertex
    SurfVertList.push_back(imstk::Vec3d(0.25, 0.25, 0.25));// centroid
    SurfVertList.push_back(imstk::Vec3d(1.05, 0, 0));
    triMesh->setInitialVerticesPositions(SurfVertList);
    triMesh->setVerticesPositions(SurfVertList);

    // Construct a map
    auto tetTriMap = std::make_shared<imstk::TetraTriangleMap>();
    tetTriMap->setMaster(tetMesh);
    tetTriMap->setSlave(triMesh);
    tetTriMap->compute();

    tetTriMap->print();

    getchar();
}

void testExtractSurfaceMesh()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(1.0, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1.0, 0));
    vertList.push_back(imstk::Vec3d(0, 0, 1.0));
    vertList.push_back(imstk::Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVerticesPositions(vertList);
    tetMesh->setVerticesPositions(vertList);

    // a.2 add connectivity
    std::vector<imstk::TetrahedralMesh::TetraArray> tetConnectivity;
    imstk::TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    imstk::TetrahedralMesh::TetraArray tet2 = { 1, 2, 3, 4 };
    tetConnectivity.push_back(tet1);
    tetConnectivity.push_back(tet2);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // b. Print tetrahedral mesh
    tetMesh->print();

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    if (tetMesh->extractSurfaceMesh(surfMesh))
    {
        // c.1. Print the resulting mesh
        surfMesh->print();
    }
    else
    {
        LOG(WARNING) << "Surface mesh was not extracted!";
    }

    getchar();
}

void testOneToOneNodalMap()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<imstk::TetrahedralMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(1.0, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1.0, 0));
    vertList.push_back(imstk::Vec3d(0, 0, 1.0));
    vertList.push_back(imstk::Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVerticesPositions(vertList);
    tetMesh->setVerticesPositions(vertList);

    tetMesh->print();

    // b. Construct a surface mesh
    auto triMesh = std::make_shared<imstk::SurfaceMesh>();

    // b.1 Add vertex positions
    std::vector<imstk::Vec3d> SurfVertList;
    SurfVertList.push_back(imstk::Vec3d(0, 0, 0));
    SurfVertList.push_back(imstk::Vec3d(1.0, 0, 0));
    SurfVertList.push_back(imstk::Vec3d(0, 1.0, 0));
    SurfVertList.push_back(imstk::Vec3d(0, 0, 1.0));
    SurfVertList.push_back(imstk::Vec3d(1.0, 1.0, 1.0));
    triMesh->setInitialVerticesPositions(SurfVertList);
    triMesh->setVerticesPositions(SurfVertList);

    // b.2 Add vertex connectivity
    std::vector<imstk::SurfaceMesh::TriangleArray> triConnectivity;
    triConnectivity.push_back({ { 0, 1, 2 } });
    triConnectivity.push_back({ { 0, 1, 3 } });
    triConnectivity.push_back({ { 0, 2, 3 } });
    triConnectivity.push_back({ { 1, 2, 4 } });
    triConnectivity.push_back({ { 1, 3, 4 } });
    triConnectivity.push_back({ { 2, 3, 4 } });
    triMesh->setTrianglesVertices(triConnectivity);

    triMesh->print();

    // c. Construct the one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<imstk::OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(triMesh);

    // d. Compute the map
    oneToOneNodalMap->compute();

    // e. Print the computed nodal map if valid
    if (oneToOneNodalMap->isValid())
    {
        oneToOneNodalMap->print();
    }

    getchar();
}

void testSurfaceMeshOptimizer()
{
    auto sdk = std::make_shared<imstk::SimulationManager>();

    // a. Construct a sample triangular mesh

    // b. Add nodal data
    auto surfMesh = std::make_shared<imstk::SurfaceMesh>();
    std::vector<imstk::Vec3d> vertList;
    vertList.push_back(imstk::Vec3d(0, 0, 0));
    vertList.push_back(imstk::Vec3d(0.5, 0.5, 0));
    vertList.push_back(imstk::Vec3d(1, 1, 0));
    vertList.push_back(imstk::Vec3d(1, 0, 0));
    vertList.push_back(imstk::Vec3d(0, 1, 0));
    vertList.push_back(imstk::Vec3d(0.5, 1, 0));
    vertList.push_back(imstk::Vec3d(0, 0.5, 0));
    vertList.push_back(imstk::Vec3d(1, 0.5, 0));
    vertList.push_back(imstk::Vec3d(0.5, 0, 0));
    surfMesh->setInitialVerticesPositions(vertList);
    surfMesh->setVerticesPositions(vertList);

    // c. Add connectivity data
    std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
    imstk::SurfaceMesh::TriangleArray tri[8];
    tri[0] = { { 0, 8, 6 } };
    tri[1] = { { 7, 2, 5 } };
    tri[2] = { { 1, 5, 4 } };
    tri[3] = { { 3, 7, 1 } };
    tri[4] = { { 8, 1, 6 } };
    tri[5] = { { 1, 4, 6 } };
    tri[6] = { { 1, 7, 5 } };
    tri[7] = { { 3, 1, 8 } };

    for (int i = 0; i < 8; i++)
    {
        triangles.push_back(tri[i]);
    }

    surfMesh->setTrianglesVertices(triangles);

    // d. Print the mesh
    surfMesh->print();

    // e. Rewire the mesh position and connectivity
    surfMesh->optimizeForDataLocality();

    // f. Print the resulting mesh
    surfMesh->print();

    // Cross-check
    // Connectivity: 0:(0, 1, 2), 1:(1, 3, 2), 2:(3, 4, 2), 3:(5, 3, 1), 4:(3, 6, 4), 5:(5, 7, 3), 6:(3, 7, 6), 7:(7, 8, 6)
    // Nodal data: 0:(0, 0, 0), 1:(0.5, 0, 0), 2:(0, 0.5, 0), 3:(0.5, 0.5, 0), 4:(0, 1, 0), 5:(1, 0, 0), 6:(0.5, 1, 0), 7:(1, 0.5, 0), 8:(1, 1, 0)

    getchar();
}
