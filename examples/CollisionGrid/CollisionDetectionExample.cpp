#include "CollisionDetectionExample.h"
#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smMesh/smLattice.h"
#include "smMesh/smLatticeTypes.h"
#include "smCore/smEventData.h"

void CollisionDetectionExample::initHapticCamMotion(){

	hapticInterface = new smPhantomInterface();
	hapticInterface->forceEnabled=false;//disable forces right now for all devices
	hapticInterface->startDevice();
	hapticInterface->setEventDispatcher(sofmisSDK->getEventDispatcher());
	motionTrans=new	smHapticCameraTrans (0);
	motionTrans->setMotionScale(0.1);
	sofmisSDK->getEventDispatcher()->registerEventHandler( viewer,SOFMIS_EVENTTYPE_CAMERA_UPDATE);
	viewer->enableCameraMotion=true;
}

CollisionDetectionExample::CollisionDetectionExample(){

	motionTrans=NULL;
	hapticInterface=NULL;

	//initializes the spatial grid
	spatGrid=new smSpatialGrid();

	///create the sdk
	sofmisSDK=smSDK::createSDK();
	///create scene objects
	object1=new smStaticSceneObject();
	object2=new smStaticSceneObject();

	///create a 3D lattice for each object
	lat= new smLattice();
	lat2= new smLattice();

	///create a scene
	scene1=sofmisSDK->createScene();

	///dummy simulator. it translates the object
	dummySim=new smDummySimulator(smSDK::getErrorLog());
	sofmisSDK->getEventDispatcher()->registerEventHandler(dummySim,SOFMIS_EVENTTYPE_KEYBOARD);

	///init texture manager and give the texture file names to be loaded
	smTextureManager::init(smSDK::getErrorLog());
	smTextureManager::loadTexture("../../resources/textures/fat9.bmp","livertexture1");
	smTextureManager::loadTexture("../../resources/textures/blood.jpg","livertexture2");
	
	///For rendering the ground
	smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg","groundImage");//ground decal image
	smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg","groundBumpImage");//gorund bum map image
	smTextureManager::loadTexture("../../resources/textures/brick.jpg","wallImage");//ground wall image
	smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg","wallBumpImage");//ground wall bump image

	///load a mesh
	object1->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS",SM_FILETYPE_3DS); 

	///texture attachment needed for fixed opengl rendering if texture is needed
	object1->mesh->assignTexture("livertexture1");
	object1->renderDetail.renderType=(SOFMIS_RENDER_FACES|SOFMIS_RENDER_TEXTURE|SOFMIS_RENDER_MATERIALCOLOR);
	object1->mesh->translate(7,0,0);
	object1->renderDetail.lineSize=2;
	object1->renderDetail.pointSize=5;

	///add object1 to lattice
	lat->addObject(object1);

	///add lattice to the grid
	spatGrid->addLattice(lat);

	///the similiar routines for object2
	object2=new smStaticSceneObject();
	object2->mesh->loadMeshLegacy("../../resources/models/liverNormalized_SB2.3DS",SM_FILETYPE_3DS); 
	object2->mesh->translate(smVec3<smFloat>(2,0,0));

	object2->mesh->assignTexture("livertexture2");
	object2->renderDetail.shadowColor.rgba[0]=1.0;
	object2->renderDetail.renderType=(SOFMIS_RENDER_FACES|SOFMIS_RENDER_TEXTURE|SOFMIS_RENDER_MATERIALCOLOR);

	lat2->addObject(object2);
	spatGrid->addLattice(lat2);
	spatGrid->pipe->registerListener(&this->myCollInformation);
	this->myCollInformation.listenerObject=&*this;
	this->myCollInformation.regType=SOFMIS_PIPE_BYREF;

	///register the module for spatial grid
	sofmisSDK->registerModule(spatGrid);	

	//add object to the scene
	scene1->addSceneObject(object1);
	scene1->addSceneObject(object2);

	///create the simulator
	simulator=sofmisSDK->createSimulator();
	///attach the dummy simulator
	simulator->registerObjectSimulator(dummySim);

	///create a viewer
	viewer=sofmisSDK->createViewer();

	//specify the viewer global settings
	viewer->viewerRenderDetail=viewer->viewerRenderDetail|SOFMIS_VIEWERRENDER_GROUND;
	viewer->camera()->setFieldOfView(SM_DEGREES2RADIANS(60));
	viewer->camera()->setZClippingCoefficient(1000);
	viewer->camera()->setZNearCoefficient(0.001);
	viewer->list();
	viewer->setWindowTitle("SOFMIS TEST");

	///assign dispatcher
	viewer->setEventDispatcher(sofmisSDK->getEventDispatcher());

	///You can either add object to the viewer or add object to the scene. Draw function will be called
	viewer->addObject(spatGrid);
	viewer->addObject(this);

	///run the simulation
	sofmisSDK->run();
}

CollisionDetectionExample::~CollisionDetectionExample(){

	delete object1;
	delete object2;
	delete scene1;
	delete dummySim;
	delete lat;
	delete lat2;
	delete spatGrid;

	if(motionTrans!=NULL)
		delete motionTrans;

	if(hapticInterface!=NULL)
		delete hapticInterface;
}

///Draw collided triangles
void CollisionDetectionExample::draw(smDrawParam p_params){

	smCollidedTriangles *tris;
	if(myCollInformation.data.dataReady){
		if(myCollInformation.data.nbrElements>0){
			tris=(smCollidedTriangles *)myCollInformation.data.dataLocation;
			glBegin(GL_TRIANGLES);
			for(smInt i=0;i<myCollInformation.data.nbrElements;i++){
				glVertex3fv((GLfloat*)&tris[i].tri1.vert[0]);
				glVertex3fv((GLfloat*)&tris[i].tri1.vert[1]);
				glVertex3fv((GLfloat*)&tris[i].tri1.vert[2]);

				glVertex3fv((GLfloat*)&tris[i].tri2.vert[0]);
				glVertex3fv((GLfloat*)&tris[i].tri2.vert[1]);
				glVertex3fv((GLfloat*)&tris[i].tri2.vert[2]);
			}
			glEnd();
		}
	}

}

//extern function 
void main(){
	CollisionDetectionExample *ex=new CollisionDetectionExample();
	delete ex;
}