#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smSDK.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smEventData.h"
#include <GL/glew.h>
#include <qapplication.h>
#include "smCore/smTextureManager.h"
#include "smExternalDevices/smPhantomInterface.h"
#include "smSimulators/smStylusObject.h"
#include <QVector>
#include <iostream>
#include "smShader/smShader.h"
#include "smShader/metalShader.h"
#include "smSimulators/smToolSimulator.h"
#include <stack>
#include "smSimulators/smToolSimulator.h"
#include "smTools/curvedGrasper.h"
#include "smShader/metalShader.h"

using namespace std;

void main(){
	smPhantomInterface * hapticInterface;
	smSDK* sofmisSDK;
	smScene *scene1;
	smViewer *viewer;
	smSimulator *simulator;

	///create lights
	smLight light("Light0",SOFMIS_LIGHT_INFINITELIGHT,SOFMIS_LIGHTPOS_EYE);
	light.lightColorDiffuse.setValue(0.8,0.8,0.8,1);
	light.lightColorAmbient.setValue(0.1,0.1,0.1,1);
	light.lightColorSpecular.setValue(0.9,0.0,0.0,1);
	light.spotCutOffAngle=180.0;

	light.lightPos.pos.setValue(0,2.0,0);
	light.direction=smVec3<smFloat>(0.0,0.0,1.0);
	light.drawEnabled=true;


	smLight light2("Light1",SOFMIS_LIGHT_SPOTLIGHT,SOFMIS_LIGHTPOS_WORLD);
	light2.lightColorDiffuse.setValue(0.4,0.4,0.4,1);
	light2.lightColorAmbient.setValue(0.1,0.1,0.1,1);
	light2.lightColorSpecular.setValue(0.1,0.1,0.1,1);
	light2.spotCutOffAngle=30.0;

	light2.lightPos.pos.setValue(20,20,20);
	light2.direction.setValue(-1,-1,-1);

	light2.drawEnabled=true;
	light2.castShadow=true;

	///Creat the SDK. SDK is a singleton object.
	sofmisSDK=smSDK::createSDK();
	scene1=sofmisSDK->createScene();
	scene1->setName("Scene1");

	///init texture manager and load the textures
	smTextureManager::init(sofmisSDK->getErrorLog());
	smTextureManager::loadTexture("../../resources/textures/metal.bmp","metal");
	smTextureManager::loadTexture("../../resources/textures/hook_cautery3.bmp","hookCautery");
	smTextureManager::loadTexture("../../resources/textures/metalbump.bmp","bump");
	smTextureManager::loadTexture("../../resources/textures/sword_bump.bmp","nobump");
	smTextureManager::loadTexture("../../resources/textures/burn1024.bmp","specTex");///for OCC I'll use the same texture

	///initialize the vertex and fragment shader
	MetalShaderShadow *metalShader=new MetalShaderShadow(
		"../../resources/shaders/MultipleShadowsVertexBumpMap2.cg",
		"../../resources/shaders/MultipleShadowsFragmentBumpMap2.cg");

	///register the shader. Otherwise it won't initialize
	metalShader->registerShader();

	///tools are maniuplated with smToolSimulator
	smToolSimulator *toolSim = new smToolSimulator(smSDK::getErrorLog());

	///curved grasper requires device ID and models for pivot, upper and lower mesh
	curvedGrasper *curvedTool;
	curvedTool = new curvedGrasper(0,"../../resources/models/curved_pivot.3DS",
									"../../resources/models/curved_upper.3DS",
									"../../resources/models/curved_lower.3DS"); 

	///textures assignments are necessary if shading will not be used
	curvedTool->mesh_pivot->assignTexture("toolPivot");
	curvedTool->mesh_lowerJaw->assignTexture("toolJaw");
	curvedTool->mesh_upperJaw->assignTexture("toolJaw");

	///we want to use shader. This requires the unique shader ID
	curvedTool->renderDetail.addShader(metalShader->uniqueId);

	///prevent casting shading on itself
	curvedTool->mesh_lowerJaw->renderDetail.canGetShadow=false;
	curvedTool->mesh_upperJaw->renderDetail.canGetShadow=false;
	curvedTool->mesh_lowerJaw->renderDetail.shininess=5.0;
	curvedTool->mesh_upperJaw->renderDetail.shininess=5.0;
	curvedTool->mesh_pivot->renderDetail.shininess=5.0;
	curvedTool->mesh_lowerJaw->scale(smVec3<float>(1.0,1.0,1.5));
	curvedTool->mesh_upperJaw->scale(smVec3<float>(1.0,1.0,1.5));

	///Each mesh is attached with proper shader texture bindings 
	metalShader->attachMesh(curvedTool->mesh_lowerJaw,"bump","metal","specTex","specTex","specTex");
	metalShader->attachMesh(curvedTool->mesh_upperJaw,"bump","metal","specTex","specTex","specTex");
	metalShader->attachMesh(curvedTool->mesh_pivot,"bump","hookCautery","specTex","specTex","specTex");

	///Transformation hiearhcy is defined with the meshes 
	curvedTool->addMeshContainer(&curvedTool->meshContainer_pivot);
	curvedTool->addMeshContainer(curvedTool->meshContainer_pivot.name, &curvedTool->meshContainer_lowerJaw);
	curvedTool->addMeshContainer(curvedTool->meshContainer_pivot.name, &curvedTool->meshContainer_upperJaw);
	curvedTool->renderDetail.renderType = (SOFMIS_RENDER_FACES|SOFMIS_RENDER_TEXTURE|SOFMIS_RENDER_MATERIALCOLOR);
	scene1->addSceneObject(curvedTool);

	///Tool is attached to Tool simulator
	curvedTool->attachObjectSimulator(toolSim);	

	///A seperate thread will be spawned for the tool simulator
	toolSim->setExecutionType(SOFMIS_SIMEXECUTION_ASYNCMODE);

	///create the main simulator
	simulator = sofmisSDK->createSimulator();

	///register the tool simulator with the simulator.
	simulator->registerObjectSimulator(toolSim);

	///create viewer
	viewer=sofmisSDK->createViewer();
	viewer->viewerRenderDetail=viewer->viewerRenderDetail;

	viewer->list();
	viewer->setWindowTitle("SOFMIS TEST");
	viewer->viewerRenderDetail=viewer->viewerRenderDetail|SOFMIS_VIEWERRENDER_GLOBALAXIS|
					SOFMIS_VIEWERRENDER_SOFTSHADOWS|SOFMIS_VIEWERRENDER_RESTORELASTCAMSETTINGS;
	viewer->viewerRenderDetail=viewer->viewerRenderDetail;
	viewer->camera()->setZClippingCoefficient(100);
	viewer->setEventDispatcher(sofmisSDK->getEventDispatcher());

	///add lights for static opengl rendering
	viewer->addLight(&light);
	viewer->addLight(&light2);

	///register event and event handlers
	sofmisSDK->getEventDispatcher()->registerEventHandler( viewer,SOFMIS_EVENTTYPE_HAPTICOUT);
	sofmisSDK->getEventDispatcher()->registerEventHandler( viewer,SOFMIS_EVENTTYPE_HAPTICIN);
	sofmisSDK->getEventDispatcher()->registerEventHandler( viewer,SOFMIS_EVENTTYPE_CAMERA_UPDATE);
	sofmisSDK->getEventDispatcher()->registerEventHandler(curvedTool, SOFMIS_EVENTTYPE_HAPTICOUT);

	///create a phantom interface
	hapticInterface = new smPhantomInterface();

	///set dispacther. This will be used to distrubute events
	hapticInterface->setEventDispatcher(sofmisSDK->getEventDispatcher());

	///register the haptic interface module
	sofmisSDK->registerModule(hapticInterface);
	viewer->addObject(hapticInterface);

	///run the SDK. SDK will initiate all modules etc.
	sofmisSDK->run();
}
