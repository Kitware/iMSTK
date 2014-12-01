#include "smSimulators/smPBDSceneObject.h"

void smPBDSurfaceSceneObject::draw(smDrawParam p_params){
	p_params.caller=this;
	mesh->draw(p_params);
}

void smPBDSurfaceSceneObject::findFixedMassWrtSphere(smVec3f p_center, smFloat p_radius){
	smFloat dist=0;
	for(smInt i=0;i<mesh->nbrVertices;i++){

		dist = (p_center- mesh->vertices[i]).module();
		if(dist<p_radius){
			fixedMass[i]=true;
		}
	}
}














