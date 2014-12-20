#include "AlphaMappingExample.h"
#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smMesh/smLattice.h"
#include "smMesh/smLatticeTypes.h"
#include "smCore/smEventData.h"

/// \brief !!
void AlphaMapExample::initHapticCamMotion(){
	hapticInterface = new smPhantomInterface();
	hapticInterface->forceEnabled=false;//disable forces right now for all devices
	hapticInterface->startDevice();
	hapticInterface->setEventDispatcher(simmedtkSDK->getEventDispatcher());
	motionTrans=new	smHapticCameraTrans (0);
	motionTrans->setMotionScale(0.1);
	simmedtkSDK->getEventDispatcher()->registerEventHandler( viewer,SIMMEDTK_EVENTTYPE_CAMERA_UPDATE);
	viewer->enableCameraMotion=true;

}

/// \brief !!
AlphaMapExample::AlphaMapExample(){

	motionTrans=NULL;
	hapticInterface=NULL;

	simmedtkSDK=smSDK::createSDK();
	object1=new smStaticSceneObject();

	MetalShader *metalShader=new MetalShader();
	metalShader->registerShader();
	scene1=simmedtkSDK->createScene();

	smTextureManager::init(smSDK::getErrorLog());
	smTextureManager::loadTexture("../../resources/textures/4351-diffuse.jpg","groundImage");
	smTextureManager::loadTexture("../../resources/textures/4351-normal.jpg","groundBumpImage");
	smTextureManager::loadTexture("../../resources/textures/brick.jpg","wallImage");
	smTextureManager::loadTexture("../../resources/textures/brick-normal.jpg","wallBumpImage");
	smTextureManager::loadTexture("../../resources/textures/Tissue.jpg","diffuse");
	smTextureManager::loadTexture("../../resources/textures/Tissue_Alpha.jpg","alpha");
	smTextureManager::loadTexture("../../resources/textures/Tissue_NORM.jpg","norm");
	smTextureManager::loadTexture("../../resources/textures/Tissue_SPEC.jpg","spec");
	smTextureManager::loadTexture("../../resources/textures/band.bmp","noOCC");

	object1->mesh->loadMeshLegacy("../../resources/models/gall_tissue.3DS",SM_FILETYPE_3DS); 

	metalShader->attachTexture(object1->mesh->uniqueId,"norm","BumpTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"diffuse","DecalTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"spec","SpecularTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"vblastDisp","DispTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"noOCC","OCCTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"alpha","AlphaTex");

	object1->renderDetail.renderType=(SIMMEDTK_RENDER_FACES|SIMMEDTK_RENDER_TEXTURE|
							SIMMEDTK_RENDER_MATERIALCOLOR|SIMMEDTK_RENDER_TRANSPARENT);
	object1->renderDetail.addShader(metalShader->uniqueId);
	object1->mesh->translate(0,10,-100);
	object1->mesh->scale(smVec3f(2,2,2));
	object1->renderDetail.lineSize=2;
	object1->renderDetail.pointSize=5;

	/// add object to the scene
	scene1->addSceneObject(object1);

	viewer=simmedtkSDK->createViewer();	
	viewer->viewerRenderDetail=viewer->viewerRenderDetail|SIMMEDTK_VIEWERRENDER_GROUND;
	viewer->list();
	viewer->setWindowTitle("SimMedTK TEST");

	viewer->setEventDispatcher(simmedtkSDK->getEventDispatcher());
	viewer->camera()->setZClippingCoefficient(1000);
	viewer->camera()->setZNearCoefficient(0.001);
	viewer->list();
	viewer->addObject(this);
	viewer->addObject(metalShader);

	simmedtkSDK->run();
}

/// \brief !!
AlphaMapExample::~AlphaMapExample(){

	delete object1;
	delete scene1;

	if(motionTrans!=NULL)
		delete motionTrans;

	if(hapticInterface!=NULL)
		delete hapticInterface;
}

/// \brief !!
void AlphaMapExample::draw(smDrawParam p_params){
}

//extern function 
void main(){
	AlphaMapExample *ex=new AlphaMapExample();
	delete ex;
	
}
