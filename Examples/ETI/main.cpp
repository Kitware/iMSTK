#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>

#include "imstkMath.h"
#include "imstkSimulationManager.h"

#include "imstkMeshReader.h"
#include "imstkSurfaceMesh.h"
#include "g3log/g3log.hpp"

// Devices
#include "imstkHDAPIDeviceClient.h"

#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkVirtualCouplingObject.h"

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
	bool loadScene = true;
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


	//// Sphere0
	//auto sphere0Geom = std::make_shared<imstk::Sphere>();
	//sphere0Geom->setPosition(imstk::Vec3d(0, 0, 0));
	//sphere0Geom->scale(1);
	//auto sphere0Obj = std::make_shared<imstk::VirtualCouplingObject>("Sphere0", client0, 0.05);
	//sphere0Obj->setVisualGeometry(sphere0Geom);
	//sphere0Obj->setCollidingGeometry(sphere0Geom);
	//scene->addSceneObject(sphere0Obj);

	bool coarseMesh = true;

	std::string path2obj;

	if (coarseMesh)
		path2obj = "resources/Tools/handle2.obj";
	else
		path2obj = "resources/Tools/handle.obj";
	
	auto mesh = imstk::MeshReader::read(path2obj);

	auto meshObj = std::make_shared<imstk::VirtualCouplingObject>("mesh", client0, 0.05);

	mesh->setPosition(imstk::Vec3d(0, 0, 0));	
	mesh->scale(0.1);
	meshObj->setVisualGeometry(mesh);
	meshObj->setCollidingGeometry(mesh);
	scene->addSceneObject(meshObj);

	if (coarseMesh)
		path2obj = "resources/Tools/blade2.obj";
	else
		path2obj = "resources/Tools/blade.obj";

	auto mesh1 = imstk::MeshReader::read(path2obj);
	auto meshObj1 = std::make_shared<imstk::VirtualCouplingObject>("mesh1", client0, 0.05);
	meshObj1->setCollidingGeometry(mesh1);
	mesh1->scale(0.1);
//	mesh1->rotate(Vec3d(0.0, 0.0, 1.0), -PI / 2);
	mesh1->setPosition(imstk::Vec3d(0, 0, 0));
	meshObj1->setVisualGeometry(mesh1);
	
	scene->addSceneObject(meshObj1);

	//_staticORObjectMesh.push_back(imstk::MeshReader::read("resources/OperatingRoom/bed1.obj"));
	//_staticORObject.push_back(std::make_shared<imstk::VisualObject>("bed1"));
	//textureName.push_back("resources/TextureOR/bed-1.jpg");

	/*
	auto mesh3 = imstk::MeshReader::read("resources/human/teeth.obj");
	auto obj3 = std::make_shared<imstk::VisualObject>("obj3");
	obj3->setVisualGeometry(mesh3);
	obj3->setCollidingGeometry(mesh3);
	scene->addSceneObject(obj3);

	auto mesh4 = imstk::MeshReader::read("resources/human/trachea.obj");
	auto obj4 = std::make_shared<imstk::VisualObject>("obj4");
	obj4->setVisualGeometry(mesh4);
	obj4->setCollidingGeometry(mesh4);
	scene->addSceneObject(obj4);
	*/
	


	/*
	auto mesh6 = imstk::MeshReader::read("resources/human/dude_111.obj");
	auto obj6 = std::make_shared<imstk::VisualObject>("obj6");
	obj6->setVisualGeometry(mesh6);
	obj6->setCollidingGeometry(mesh6);
	scene->addSceneObject(obj6);
	*/
	//	auto mesh2 = imstk::MeshReader::read("/home/virtualfls/Projects/IMSTK/resources/Spheres/small_0.vtk");



	sdk->setCurrentScene("ETI simulator");
    sdk->startSimulation(false);
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