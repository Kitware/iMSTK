#include "smSimulators/smPBDSceneObject.h"
#include "smSimulators/smPBDObjectSimulator.h"
#include "smRendering/smGLRenderer.h"

//
void smPBDObjectSimulator::draw(smDrawParam p_params)
{
	smObjectSimulator::draw(p_params);
	smPBDSurfaceSceneObject *sceneObject;
	for(smInt i=0;i<objectsSimulated.size();i++)
	{
		sceneObject=(smPBDSurfaceSceneObject*)objectsSimulated[i];	
		smGLRenderer::draw(sceneObject->mesh->aabb);

	}
	
}