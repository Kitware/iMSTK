#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkMath.h"
#include "imstkSimulationManager.h"

#include "imstkMeshReader.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "g3log/g3log.hpp"


// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

// Devices
#include "imstkHDAPIDeviceClient.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkVirtualCouplingObject.h"
#include "imstkVirtualCouplingPBDObject.h"
// Collisions
#include "imstkInteractionPair.h"

#include "imstkPbdObject.h"

#include "ETI.h"

using namespace imstk;

typedef std::vector<std::shared_ptr<Mesh> > MeshList;
typedef std::vector<std::shared_ptr<SurfaceMesh> > SurfaceMeshList;
typedef std::vector<std::shared_ptr<imstk::VisualObject> > VisualObjectList;

void initializeText();
void loadSceneTexture(int nbrSceneTexture);
void loadModelTexture();
void loadSimulationTexture();
void initializeDeformableObjects();
void initializeTools();
void initializeSkinBone();
void initializeHapticDevice();
void initializeOR(MeshList _staticORObjectMesh, SurfaceMeshList _staticORObjectSurfaceMesh, VisualObjectList _staticORObject, std::shared_ptr<Scene> _scene);
void initializeHumanModel(MeshList _staticModelMesh, SurfaceMeshList _staticModelSurfaceMesh, VisualObjectList _staticModel, std::shared_ptr<Scene> _scene);



int main()
{
	auto sdk = std::make_shared<SimulationManager>();
	auto scene = sdk->createNewScene("ETI simulator");
	bool loadScene = false;
	bool loadModel = false;
	
	// initialize text record
	// initializeText();

	// load static objects(and texture) in OR
	MeshList staticORObjectMesh;
	SurfaceMeshList staticORObjectSurfaceMesh;
	VisualObjectList staticORObject;
	if (loadScene)
		initializeOR(staticORObjectMesh, staticORObjectSurfaceMesh, staticORObject, scene);

	// load human models(and texture)
	MeshList modelMesh;
	SurfaceMeshList staticModelSurfaceMesh;
	VisualObjectList model;
	if (loadModel)
		initializeHumanModel(modelMesh, staticModelSurfaceMesh, model, scene);

	// Device clients
	auto client0 = std::make_shared<imstk::HDAPIDeviceClient>("PHANToM 1");
	sdk->addDeviceClient(client0);

	bool coarseMesh = true;

	std::string path2obj;

	if (coarseMesh)
		path2obj = "resources/Tools/handle2.obj";
	else
		path2obj = "resources/Tools/handle.obj";
	
	auto mesh = imstk::MeshReader::read(path2obj);
	auto visualMesh = imstk::MeshReader::read(path2obj);
	
	auto handle = std::make_shared<imstk::VirtualCouplingPBDObject>("handle", client0, 0.5);

	auto oneToOneHandle = std::make_shared<imstk::OneToOneMap>();
	oneToOneHandle->setMaster(mesh);
	oneToOneHandle->setSlave(mesh);
	oneToOneHandle->compute();

	auto C2VHandle = std::make_shared<imstk::OneToOneMap>();
	C2VHandle->setMaster(mesh);
	C2VHandle->setSlave(visualMesh);
	C2VHandle->compute();

	handle->setCollidingGeometry(mesh);
	handle->setVisualGeometry(visualMesh);
	handle->setPhysicsGeometry(mesh);
	handle->setPhysicsToCollidingMap(oneToOneHandle);
	handle->setCollidingToVisualMap(C2VHandle);
	handle->setPhysicsToVisualMap(oneToOneHandle);
	handle->init(/*Number of constraints*/0,
		/*Mass*/0.0,
		/*Gravity*/"0 0 0",
		/*TimeStep*/0.001,
		/*FixedPoint*/"",
		/*NumberOfIterationInConstraintSolver*/5,
		/*Proximity*/0.1,
		/*Contact stiffness*/0.01);
	scene->addSceneObject(handle);


	if (coarseMesh)
		path2obj = "resources/Tools/blade2.obj";
	else
		path2obj = "resources/Tools/blade.obj";

	auto mesh1 = imstk::MeshReader::read(path2obj);
	auto viusalMesh1 = imstk::MeshReader::read(path2obj);
	auto blade = std::make_shared<imstk::VirtualCouplingPBDObject>("blade", client0, 0.5);

	auto oneToOneBlade = std::make_shared<imstk::OneToOneMap>();
	oneToOneBlade->setMaster(mesh1);
	oneToOneBlade->setSlave(mesh1);
	oneToOneBlade->compute();

	auto C2VBlade = std::make_shared<imstk::OneToOneMap>();
	C2VBlade->setMaster(mesh1);
	C2VBlade->setSlave(viusalMesh1);
	C2VBlade->compute();

	blade->setCollidingGeometry(mesh1);
	blade->setVisualGeometry(viusalMesh1);
	blade->setPhysicsGeometry(mesh1);
	blade->setPhysicsToCollidingMap(oneToOneBlade);
	blade->setCollidingToVisualMap(C2VBlade);
	blade->setPhysicsToVisualMap(C2VBlade);
	blade->init(/*Number of constraints*/0,
		/*Mass*/0.0,
		/*Gravity*/"0 0 0",
		/*TimeStep*/0.001,
		/*FixedPoint*/"",
		/*NumberOfIterationInConstraintSolver*/5,
		/*Proximity*/0.1,
		/*Contact stiffness*/0.01);
	scene->addSceneObject(blade);

//	scene->addSceneObject(meshObj1);


	// floor
	auto floorMesh = std::make_shared<imstk::SurfaceMesh>();
	std::vector<imstk::Vec3d> vertList;
	double width = 60.0;
	double height = 60.0;
	int nRows = 20;
	int nCols = 20;
	int corner[4] = { 1, nRows, nRows*nCols - nCols+1,nRows*nCols };
	char intStr[33];
	std::string fixed_corner;
	for (unsigned int i = 0; i < 4; i++){
		itoa(corner[i], intStr, 10);
		fixed_corner += std::string(intStr) + ' ';
	}
	vertList.resize(nRows*nCols);
	const double dy = width / (double)(nCols - 1);
	const double dx = height / (double)(nRows - 1);
	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			const double y = (double)dy*j;
			const double x = (double)dx*i;
			vertList[i*nCols + j] = Vec3d(x - 30, -25, y-60);

		}
	}
	floorMesh->setInitialVerticesPositions(vertList);
	floorMesh->setVerticesPositions(vertList);

	// c. Add connectivity data
	std::vector<imstk::SurfaceMesh::TriangleArray> triangles;
	for (int i = 0; i < nRows - 1; i++)
	{
		for (int j = 0; j < nCols - 1; j++)
		{
			imstk::SurfaceMesh::TriangleArray tri[2];
			tri[0] = { { i*nCols + j, i*nCols + j + 1, (i + 1)*nCols + j } };
			tri[1] = { { (i + 1)*nCols + j + 1, (i + 1)*nCols + j, i*nCols + j + 1 } };
			triangles.push_back(tri[0]);
			triangles.push_back(tri[1]);
		}		
	}
	floorMesh->setTrianglesVertices(triangles);

	auto oneToOneFloor = std::make_shared<imstk::OneToOneMap>();
	oneToOneFloor->setMaster(floorMesh);
	oneToOneFloor->setSlave(floorMesh);
	oneToOneFloor->compute();

	auto floor = std::make_shared<PbdObject>("Floor");
	floor->setCollidingGeometry(floorMesh);
	floor->setVisualGeometry(floorMesh);
	floor->setPhysicsGeometry(floorMesh);
	floor->setPhysicsToCollidingMap(oneToOneFloor);
	floor->setPhysicsToVisualMap(oneToOneFloor);
	floor->setCollidingToVisualMap(oneToOneFloor);
	floor->init(/*Number of constraints*/2,
		/*Constraint configuration*/"Distance 0.1",
		/*Constraint configuration*/"Dihedral 0.001",
		/*Mass*/0.5,
		/*Gravity*/"0 -9.8 0",
		/*TimeStep*/0.01,
		/*FixedPoint*/fixed_corner.c_str(),		
		/*NumberOfIterationInConstraintSolver*/5,
		/*Proximity*/0.1,
		/*Contact stiffness*/0.01);
	scene->addSceneObject(floor);

	// Collisions
	auto colGraph = scene->getCollisionGraph();
	auto pair1 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(blade, floor));
	auto pair2 = std::make_shared<PbdInteractionPair>(PbdInteractionPair(handle, floor));
	pair1->setNumberOfInterations(5);
	pair2->setNumberOfInterations(5);

	colGraph->addInteractionPair(pair1);
	colGraph->addInteractionPair(pair2);

	Vec3d cameraPos = scene->getCamera()->getPosition();
	std::cout << cameraPos << std::endl;
	scene->getCamera()->setPosition(0,0,50);
	sdk->setCurrentScene("ETI simulator");
    sdk->startSimulation(true);
}

void initializeText()
{
	std::string line;
	ifstream record("Data/read.txt");

	if (record.is_open())
	{
		while (getline(record, line))
		{
			cout << "Last # was " << line << '\n';
		}
		record.close();
	}
	else cout << "Last # was 0" << endl;

	cout << "ID Number:" << endl;

	static std::string IDNumber;
	getline(cin, IDNumber);

	std::string PersonName1 = IDNumber;

	std::string readfile = "Data/read.txt";
	FILE *output8 = fopen(readfile.c_str(), "w");
	assert(output8);
	std::string myString = PersonName1;
	int writethis = atoi(myString.c_str());
	fprintf(output8, "%i \n", writethis);
	fclose(output8);


	time_t ltime;
	struct tm *Tm;

	ltime = time(NULL);
	Tm = localtime(&ltime);

	float number = Tm->tm_mday;
	float number1 = Tm->tm_hour;
	float number2 = Tm->tm_min;
	float number3 = Tm->tm_sec;

	std::ostringstream buff;
	buff << number;
	buff.str();

	std::ostringstream buff1;
	buff1 << number1;
	buff1.str();

	std::ostringstream buff2;
	buff2 << number2;
	buff2.str();

	std::ostringstream buff3;
	buff3 << number3;
	buff3.str();
	IDNumber = PersonName1 + " " + buff.str() + " " + buff1.str() + " " + buff2.str() + " " + buff3.str();
	std::string landmarkfile = "Data/" + IDNumber + "_ETI(Head_and_Neck_Rot).txt";
	std::string landmarkfile1 = "Data/" + IDNumber + "_ETI(Head_Rot).txt";
	std::string headmounted = "Data/" + IDNumber + "_ETI(HMD_Orientation).txt";;
	std::string hmdpos = "Data/" + IDNumber + "_ETI(HMD_Position).txt";;
	std::string angleprint = "Data/" + IDNumber + "_ETI(Skinning).txt";;
	FILE *output2 = fopen(landmarkfile.c_str(), "w");
	FILE *output4 = fopen(headmounted.c_str(), "w");
	FILE *output5 = fopen(hmdpos.c_str(), "w");
	FILE *output6 = fopen(angleprint.c_str(), "w");
	//output2 =  fopen(headmounted.c_str(), "w");
	FILE *output7 = fopen(landmarkfile1.c_str(), "w");


}

void loadSceneTexture(){

}

void initializeOR(MeshList _staticORObjectMesh, SurfaceMeshList _staticORObjectSurfaceMesh, VisualObjectList _staticORObject, std::shared_ptr<Scene> _scene){

	std::vector<std::string> textureName;

	_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/bed1.obj"));
	_staticORObject.push_back(std::make_shared<imstk::VisualObject>("bed1"));
	textureName.push_back("resources/TextureOR/bed-1.jpg");

	_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/bed2.obj"));
	_staticORObjectMesh[_staticORObjectMesh.size() - 1]->translate(0.0, 0.0, -10.0);
	_staticORObject.push_back(std::make_shared<imstk::VisualObject>("bed2"));	
	textureName.push_back("resources/TextureOR/bed-2.jpg");

	_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/bed3.obj"));
	_staticORObjectMesh[_staticORObjectMesh.size() - 1]->translate(0.0, 0.0, -10.0);
	_staticORObject.push_back(std::make_shared<imstk::VisualObject>("bed3"));
	textureName.push_back("resources/TextureOR/bed-3.jpg");

	_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/bed4.obj"));
	_staticORObjectMesh[_staticORObjectMesh.size() - 1]->translate(0.0, 0.0, -10.0);
	_staticORObject.push_back(std::make_shared<imstk::VisualObject>("bed4"));
	textureName.push_back("resources/TextureOR/bed-4.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/ceiling.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("ceiling"));
	//textureName.push_back("resources/TextureOR/ceiling.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/ceilingframe.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("ceilingframe"));
	//textureName.push_back("resources/TextureOR/ceiling frame.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/ceilinglight.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("ceilinglight"));
	//textureName.push_back("resources/TextureOR/ceiling light.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/floor.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("floor"));
	//textureName.push_back("resources/TextureOR/floor.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/infusionpole.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("infusionpole"));
	//textureName.push_back("resources/TextureOR/infusion pole.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/instrument_cart.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("instrument_cart"));
	//textureName.push_back("resources/TextureOR/instrument cart.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor1.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor1"));
	//textureName.push_back("resources/TextureOR/light_monitor-1.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor2.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor2"));
	//textureName.push_back("resources/TextureOR/light_monitor-2.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor3.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor3"));
	//textureName.push_back("resources/TextureOR/light_monitor-3.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor4.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor4"));
	//textureName.push_back("resources/TextureOR/light_monitor-4.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor5.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor5"));
	//textureName.push_back("resources/TextureOR/light_monitor-5.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor6.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor6"));
	//textureName.push_back("resources/TextureOR/light_monitor-6.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/light_monitor7.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("light_monitor7"));
	//textureName.push_back("resources/TextureOR/light_monitor-7.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/monitor1.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("monitor1"));
	//textureName.push_back("resources/TextureOR/monitor-1.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/monitor2.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("monitor2"));
	//textureName.push_back("resources/TextureOR/monitor-2.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/monitor3.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("monitor3"));
	//textureName.push_back("resources/TextureOR/monitor-3.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/monitor4.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("monitor4"));
	//textureName.push_back("resources/TextureOR/monitor-4.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/monitor5.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("monitor5"));
	//textureName.push_back("resources/TextureOR/monitor-5.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/operationlight1.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("operationlight1"));
	//textureName.push_back("resources/TextureOR/operation light-1.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/operationlight2.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("operationlight2"));
	//textureName.push_back("resources/TextureOR/operation light-2.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/operationlight3.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("operationlight3"));
	//textureName.push_back("resources/TextureOR/operation light-3.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor1.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor1"));
	//textureName.push_back("resources/TextureOR/w_monitor-1.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor2.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor2"));
	//textureName.push_back("resources/TextureOR/w_monitor-2.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor3.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor3"));
	//textureName.push_back("resources/TextureOR/w_monitor-3.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor4.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor4"));
	//textureName.push_back("resources/TextureOR/w_monitor-4.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor5.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor5"));
	//textureName.push_back("resources/TextureOR/w_monitor-5.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor6.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor6"));
	//textureName.push_back("resources/TextureOR/w_monitor-6.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor7.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor7"));
	//textureName.push_back("resources/TextureOR/w_monitor-7.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/w_monitor8.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("w_monitor8"));
	//textureName.push_back("resources/TextureOR/w_monitor-8.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/wall.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("wall"));
	//textureName.push_back("resources/TextureOR/wall.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/wallcurtain.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("wallcurtain"));
	//textureName.push_back("resources/TextureOR/window curtain.jpg");

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/windowframe.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("windowframe"));
	//textureName.push_back("resources/TextureOR/window_frame.jpg");


	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/cloth.obj"));
	//_staticORObjectMesh[_staticORObjectMesh.size() - 1]->translate(0.0, 12.0, 6.0);
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("cloth"));
	//textureName.push_back("resources/TextureOR/cloth.jpg");

	std::cout << "Number of OR scene object: " << _staticORObjectMesh.size() << std::endl;
//	std::shared_ptr<imstk::SurfaceMesh> surfaceMesh;
	for (unsigned int i = 0; i < _staticORObjectMesh.size(); i++){
		_staticORObjectSurfaceMesh.push_back(std::dynamic_pointer_cast<imstk::SurfaceMesh>(_staticORObjectMesh[i]));
//		if (i == (_staticORObjectMesh.size()-1))
		_staticORObjectSurfaceMesh[i]->addTexture(textureName[i]);
		_staticORObject[i]->setVisualGeometry(_staticORObjectSurfaceMesh[i]);
		_scene->addSceneObject(_staticORObject[i]);
	}



}

void initializeHumanModel(MeshList _staticModelMesh, SurfaceMeshList _staticModelSurfaceMesh, VisualObjectList _staticModel, std::shared_ptr<Scene> _scene){

	std::vector<std::string> textureName;

	//_staticModelMesh.push_back(imstk::MeshReader::read("resources/Human/second_guy_tongue_SB.obj"));
	//_staticModel.push_back(std::make_shared<imstk::VisualObject>("tongue"));


	_staticModelMesh.push_back(imstk::MeshReader::read("resources/Human/head.obj"));
	_staticModel.push_back(std::make_shared<imstk::VisualObject>("head"));
	textureName.push_back("resources/TextureOR/cloth.jpg");

	//_staticModelMesh.push_back(imstk::MeshReader::read("resources/Human/teeth.obj"));
	//_staticModel.push_back(std::make_shared<imstk::VisualObject>("teeth"));

	//_staticModelMesh.push_back(imstk::MeshReader::read("resources/Human/tracheal.obj"));
	//_staticModel.push_back(std::make_shared<imstk::VisualObject>("tracheal"));
	
	
	
//	std::shared_ptr<imstk::SurfaceMesh> surfaceMesh;
	for (unsigned int i = 0; i < _staticModel.size(); i++){
		_staticModelSurfaceMesh.push_back(std::dynamic_pointer_cast<imstk::SurfaceMesh>(_staticModelMesh[i]));
		_staticModelSurfaceMesh[i]->addTexture(textureName[i]);
		_staticModel[i]->setVisualGeometry(_staticModelSurfaceMesh[i]);
		_scene->addSceneObject(_staticModel[i]);
	}



	/*
	if(loadmodel)
	{
	trachealaxis->mesh->loadMesh("models/VAST_Models/VAST/a/tracheal_v7.3DS",SM_FILETYPE_3DS);
	oralaxis->mesh->loadMesh("models/VAST_Models/VAST/oral.3DS",SM_FILETYPE_3DS);
	body9->mesh->loadMesh("models/VAST_Models/class1/Head.3DS",SM_FILETYPE_3DS);
	body2->mesh->loadMesh("models/cerial/male_head_4.3DS",SM_FILETYPE_3DS);
	teeth->mesh->loadMesh("models/cerial/teeth_11_10.3DS",SM_FILETYPE_3DS);
	teeth2->mesh->loadMesh("models/cerial/trachea_11_10.3DS",SM_FILETYPE_3DS);
	tongue->mesh->loadMesh("models/cerial/tounge_11_10.3DS",SM_FILETYPE_3DS);
	cartilage->mesh->loadMesh("models/VAST_Models/VAST/ytr.3DS",SM_FILETYPE_3DS);//cartilage

	teeth->mesh->translate(-0.4,-27.5,-11.0);

	if(loadmodel)
	{
	tongue->mesh->translate(0.0,-20.4,2.0);



	teeth->alignCenterWithAnotherSkinObject(body2);
	teeth2->alignCenterWithAnotherSkinObject(body2);

	tongue->alignCenterWithAnotherSkinObject(body2);
	oralaxis->alignCenterWithAnotherSkinObject(body2);

	}
	*/
}