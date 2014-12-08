#include "AlphaMappingExample.h"

#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "smMesh/smLattice.h"
#include "smMesh/smLatticeTypes.h"
#include "smCore/smEventData.h"




void AlphaMapExample::initHapticCamMotion(){
	hapticInterface = new smPhantomInterface();
	hapticInterface->forceEnabled=false;//disable forces right now for all devices
	hapticInterface->startDevice();
	hapticInterface->setEventDispatcher(sofmisSDK->getEventDispatcher());
	motionTrans=new	smHapticCameraTrans (0);
	motionTrans->setMotionScale(0.1);
	sofmisSDK->getEventDispatcher()->registerEventHandler( viewer,SOFMIS_EVENTTYPE_CAMERA_UPDATE);
	viewer->enableCameraMotion=true;

}

AlphaMapExample::AlphaMapExample(){

	
	motionTrans=NULL;
	hapticInterface=NULL;


	
	

	sofmisSDK=smSDK::createSDK();
  	object1=new smStaticSceneObject();		
	

	MetalShader *metalShader=new MetalShader("../../resources/shaders/VertexBumpMap1.cg","../../resources/shaders/FragmentBumpMap1.cg");
	//metalShader->checkShaderUpdate(1000);
	metalShader->registerShader();
	
	scene1=sofmisSDK->createScene();

	
	//	TCHAR szEXEPath[2048];
	//	cout << GetModuleFileName ( NULL, szEXEPath, 2048 );



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
	
	//object1->mesh->assignTexture("diffuse");
	metalShader->attachTexture(object1->mesh->uniqueId,"norm","BumpTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"diffuse","DecalTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"spec","SpecularTex");
	//metalShader->attachTexture(object1->mesh->uniqueId,"vblastDisp","DispTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"noOCC","OCCTex");
	metalShader->attachTexture(object1->mesh->uniqueId,"alpha","AlphaTex");
	
	//metalShader->checkShaderUpdate(2000);
	
	//object1->mesh->assignTexture("livertexture1");
 	object1->renderDetail.renderType=(SOFMIS_RENDER_FACES|SOFMIS_RENDER_TEXTURE|SOFMIS_RENDER_MATERIALCOLOR|SOFMIS_RENDER_TRANSPARENT);
	object1->renderDetail.addShader(metalShader->uniqueId);
	object1->mesh->translate(0,10,-110);
	object1->mesh->scale(smVec3f(5,5,5));
	object1->renderDetail.lineSize=2;
	object1->renderDetail.pointSize=5;
	
	

	

	
	
	
	
	
	//add object to the scene
	scene1->addSceneObject(object1);
	

	

	viewer=sofmisSDK->createViewer();	
	viewer->viewerRenderDetail=viewer->viewerRenderDetail|SOFMIS_VIEWERRENDER_GROUND;
	viewer->list();
    viewer->setWindowTitle("SOFMIS TEST");

    


	viewer->setEventDispatcher(sofmisSDK->getEventDispatcher());
	viewer->camera()->setZClippingCoefficient(1000);
	viewer->camera()->setZNearCoefficient(0.001);
	viewer->list();
	//viewer->addObject(&lat2);
	//viewer->addObject(spatGrid);
	viewer->addObject(this);
	viewer->addObject(metalShader);

	//initHapticCamMotion();//initiate the haptic motion with camera
	

    sofmisSDK->run();
 }

AlphaMapExample::~AlphaMapExample()
{
	delete object1;

	delete scene1;

	
	if(motionTrans!=NULL)
		delete motionTrans;
	if(hapticInterface!=NULL)
		delete hapticInterface;


}

void AlphaMapExample::draw(smDrawParam p_params){
	 


}

//extern function 
int main(){
	AlphaMapExample *ex=new AlphaMapExample();
	delete ex;
	
}
