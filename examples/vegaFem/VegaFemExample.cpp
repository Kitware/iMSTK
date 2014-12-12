#include "vegaFemExample.h"

/// \brief Create a scene with one fem object using vega library
vegaFemExample::vegaFemExample(){

	//Create an instance of the SoFMIS framework/SDK
	sofmisSDK = smSDK::createSDK();

	//Create a new scene to work in
	scene1 = sofmisSDK->createScene();

	//Create a viewer to see the scene
	viewer = sofmisSDK->createViewer();

	/// create a FEM simulator
	femSim = new smVegaFemSimulator(sofmisSDK->getErrorLog());

	/// set the dispatcher for FEM. it will be used for sending events
	femSim->setDispatcher(sofmisSDK->getEventDispatcher());
	sofmisSDK->getEventDispatcher()->registerEventHandler(femSim,SOFMIS_EVENTTYPE_HAPTICOUT);

	/// create a Vega based FEM object and attach it to the fem simulator
	femobj = new smVegaFemSceneObject(sofmisSDK->getErrorLog(),
	                       "../../../resources/vega/asianDragon/asianDragon.config");
	femobj->attachObjectSimulator(femSim);

	/// add the FEM object to the scene
	scene1->addSceneObject(femobj);

	/// create a simulator module and register FEM simulator
	simulator=sofmisSDK->createSimulator();
	simulator->registerObjectSimulator(femSim);

	/// create a viewer
	viewer->viewerRenderDetail=viewer->viewerRenderDetail|SOFMIS_VIEWERRENDER_GROUND;
	viewer->setEventDispatcher(sofmisSDK->getEventDispatcher());

	//Set some camera parameters
	viewer->camera()->setZClippingCoefficient(100);

	/// run the SDK
	sofmisSDK->run();
}

void VegaFemExample(){
	vegaFemExample *ve = new vegaFemExample();
	delete ve;
}

