#include "smCore/smStaticSceneObject.h"





 void smStaticSceneObject::draw(smDrawParam p_params){
	 p_params.caller=this;
     mesh->draw(p_params);
	/* glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	 smVec3<smFloat> v1,v2,v3,temp1,temp2,normal;
	 glDisable(GL_COLOR_MATERIAL);
	 glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,  (smGLReal*)&p_params.caller->renderDetail.colorDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,(smGLReal*)&p_params.caller->renderDetail.colorSpecular);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,(smGLReal*)&p_params.caller->renderDetail.colorAmbient);
	
	 glBegin(GL_TRIANGLES);
	 for(smInt i=0;i<mesh->nbrTriangles;i++){
		 v1=mesh->vertices[mesh->triangles[i].vert[0]];
		 v2=mesh->vertices[mesh->triangles[i].vert[1]];
		 v3=mesh->vertices[mesh->triangles[i].vert[2]];

		 temp1=v3-v1;
		 temp2=v2-v1;
		 normal=temp1.cross(temp2);
		 normal=normal.unit()*-1;
		 glColor3fv(smColor::colorWhite.toGLColor());
		 glNormal3fv((smGLFloat*)&normal);
		 glVertex3fv((smGLFloat*)&v1);
		 glVertex3fv((smGLFloat*)&v2);
		 glVertex3fv((smGLFloat*)&v3);


		

	 }
	 glEnd();
*/

}
