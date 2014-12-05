#include "smCore/smSDK.h"
#include "smCore/smTextureManager.h"
#include "RenderExample.h"

/// \brief A simple example of how to render and object using SoFMIS
/// 
/// \detail This is the default constructor, however, this is where the main 
/// program runs.  This program will create a cube with a texture pattern 
/// numbering each side of the cube, that's all it does.
RenderExample::RenderExample()
{
	//Create an instance of the SoFMIS framework/SDK
	sofmisSDK = smSDK::createSDK();


	//Create a viewer to see the scene through
	viewer = sofmisSDK->createViewer();

	//Initialize the texture manager
	smTextureManager::init(smSDK::getErrorLog());

	//Load in the texture for the cube model
	smTextureManager::loadTexture("textures/cube.png", "cubetex");

	//Load the cube model
	cube.mesh->loadMesh("models/cube.obj", SM_FILETYPE_OBJ);

	//Assign the previously loaded texture to the cube model
	cube.mesh->assignTexture("cubetex");
	//Tell SoFMIS to render the faces of the model, and the texture assigned
	cube.renderDetail.renderType = (SOFMIS_RENDER_FACES | SOFMIS_RENDER_TEXTURE);

	//Add the cube to the scene to be rendered
	scene1->addSceneObject(&cube);

	//Setup the window title in the window manager
	viewer->setWindowTitle("SOFMIS RENDER TEST");
	//Add the RenderExample object we are in to the viewer from the SoFMIS SDK
	viewer->addObject(this);
	//Set some camera parameters
	viewer->camera()->setZClippingCoefficient(1000);
	viewer->camera()->setZNearCoefficient(0.001);
	viewer->camera()->setFieldOfView(SM_DEGREES2RADIANS(60));

	//Link up the event system between the viewer and the SoFMIS SDK
	//Note: This allows some default behavior like mouse and keyboard control
	viewer->setEventDispatcher(sofmisSDK->getEventDispatcher());

	printf("***viewerRenderDetail: %#x\n", viewer->viewerRenderDetail);
	sofmisSDK->getEventDispatcher()->registerEventHandler(viewer,SOFMIS_EVENTTYPE_KEYBOARD);

	printf("****viewerRenderDetail: %#x\n", viewer->viewerRenderDetail);
	//Run the simulator framework
	sofmisSDK->run();
	printf("*****viewerRenderDetail: %#x\n", viewer->viewerRenderDetail);
}

void renderExample() {
	RenderExample *re = new RenderExample();
	delete re;
}
