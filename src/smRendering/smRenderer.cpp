#include "smRendering/smGLRenderer.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smDataStructs.h"
#include "smRendering/smViewer.h"
#include "smUtilities/smQuat.h"
#include "smRendering/smVAO.h"

smGLRenderer::smGLRenderer(){

}

void smGLRenderer::drawLineMesh(smLineMesh *p_lineMesh,smRenderDetail *renderDetail)
{
	static smFloat shadowMatrixGL[16];
	static smVec3<smFloat> origin(0,0,0);
	static smVec3<smFloat> xAxis(1,0,0);
	static smVec3<smFloat> yAxis(0,1,0);
	static smVec3<smFloat> zAxis(0,0,1);
	 
	//glDisable(GL_LIGHTING);
	if(renderDetail->renderType&SOFMIS_RENDER_NONE)
		return;
	

	
	glDisable(GL_TEXTURE_2D);

	glPointSize(renderDetail->pointSize);
	glLineWidth(renderDetail->lineSize);

	if(renderDetail->renderType&SOFMIS_RENDER_TRANSPARENT){
		glEnable (GL_BLEND); 
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
	if(renderDetail->renderType&SOFMIS_RENDER_MATERIALCOLOR){
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,  renderDetail->colorDiffuse.toGLColor());
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,renderDetail->colorSpecular.toGLColor());
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,renderDetail->colorAmbient.toGLColor());
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,renderDetail->shininess);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
		if(p_lineMesh->isMeshTextured()){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			for(smInt t=0;t<p_lineMesh->textureIds.size();t++){
				glActiveTexture(GL_TEXTURE0+t);
				smTextureManager::activateTexture(p_lineMesh->textureIds[t].textureId);
			}
		}
	}

	if(renderDetail->renderType&SOFMIS_RENDER_COLORMAP)
		glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3,smGLRealType,0,p_lineMesh->vertices);
	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
		if(p_lineMesh->isMeshTextured())		
			glTexCoordPointer(2,smGLRealType,0,p_lineMesh->texCoord);
	}

	
	if(renderDetail->renderType&SOFMIS_RENDER_FACES)
		glDrawElements(GL_LINES,p_lineMesh->nbrEdges*2,smGLUIntType,p_lineMesh->edges);

	if((renderDetail->renderType&(SOFMIS_RENDER_VERTICES))){
		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
		glDisable(GL_LIGHTING);
		glDrawElements(GL_LINES,p_lineMesh->nbrEdges*2,smGLUIntType,p_lineMesh->edges);
		glEnable(GL_LIGHTING);
		//default rendering
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}

	if(renderDetail->renderType&SOFMIS_RENDER_WIREFRAME){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glPolygonOffset (3.0, 2.0);
		glDisable(GL_LIGHTING);
		glDrawElements(GL_LINES,p_lineMesh->nbrEdges*2,smGLUIntType,p_lineMesh->edges);	
		glEnable(GL_LIGHTING);
		//default rendering
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
	if(renderDetail->renderType&SOFMIS_RENDER_LOCALAXIS){
		glEnable(GL_LIGHTING);
		//drawArrow((qglviewer::Vec *)&origin,(qglviewer::Vec *)&xAxis,2,12);
		//drawArrow((qglviewer::Vec *)&origin,(qglviewer::Vec *)&yAxis,2,12);
		//drawArrow((qglviewer::Vec *)&origin,(qglviewer::Vec *)&zAxis,2,12);
	}

	if(renderDetail->renderType&SOFMIS_RENDER_HIGHLIGHTVERTICES){
		glDisable(GL_LIGHTING);
		glColor3fv((smGLReal*)&renderDetail->highLightColor);
		glDrawArrays (GL_POINTS,0,p_lineMesh->nbrVertices);
		glEnable(GL_LIGHTING);
	}

	if(renderDetail->renderType&SOFMIS_RENDER_TRANSPARENT){
		glDisable(GL_BLEND); 
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
		if(p_lineMesh->isMeshTextured()){
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			for(smInt t=0;t<p_lineMesh->textureIds.size();t++){
				glActiveTexture(GL_TEXTURE0+t);			
				smTextureManager::disableTexture(p_lineMesh->textureIds[t].textureId);
			}
		}
	}

	if(renderDetail->renderType&SOFMIS_RENDER_COLORMAP)
		glDisableClientState(GL_COLOR_ARRAY);

	glEnable(GL_LIGHTING);
	glPointSize(1.0);
	glLineWidth(1.0);
}

//void smGLRenderer::drawSurfaceMeshTriangles(smMesh *p_surfaceMesh,smRenderDetail *renderDetail, smDrawParam p_drawParam)
//{
//	static smFloat shadowMatrixGL[16];
//	static smVec3<smFloat> origin(0,0,0);
//	static smVec3<smFloat> xAxis(1,0,0);
//	static smVec3<smFloat> yAxis(0,1,0);
//	static smVec3<smFloat> zAxis(0,0,1);
//	smShader *shader=NULL;
//	smBool shaderEnabled=false;
//
//	if(p_surfaceMesh->meshFileType==SM_FILETYPE_OBJ){
//		glDisableClientState(GL_COLOR_ARRAY);
//		glDisableClientState(GL_INDEX_ARRAY);
//		glEnableClientState(GL_VERTEX_ARRAY);
//	}
//
//	if(p_surfaceMesh->renderDetail.renderType&SOFMIS_RENDER_NONE)
//		return;
//
//	if(p_drawParam.rendererObject->renderStage!=SMRENDERSTAGE_SHADOWPASS)
//	{
//		for(smInt i=0;i<renderDetail->shaders.size();i++){
//			if(renderDetail->shaderEnable[i]){
//
//				shader=smShader::getShader(renderDetail->shaders[i]);
//				shader->enableShader();
//				
//				shader->predraw(p_surfaceMesh);
//				shaderEnabled=true;
//				//renderDetail->shaders[i].shader->enableShader();
//				//renderDetail->shaders[i].shader->preDraw(this);
//				break;
//			}
//		}
//	}
//	else{
//		smGLRenderer::enableDefaultGLRendering();
//	}
//	
//	glDisable(GL_TEXTURE_2D);
//
//	glPointSize(renderDetail->pointSize);
//	glLineWidth(renderDetail->lineSize);
//
//	if(p_surfaceMesh->vertTangents!=NULL&&shaderEnabled&&p_surfaceMesh->tangentChannel){
//		glVertexAttribPointerARB(shader->tangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, p_surfaceMesh->vertTangents);
//		glEnableVertexAttribArrayARB(shader->tangentAttrib);
//	}
//
//	if(renderDetail->renderType&SOFMIS_RENDER_TRANSPARENT){
//		glEnable (GL_BLEND); 
//		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		//glBlendFunc (GL_ONE, GL_ONE);
//
//	}
//	if(renderDetail->renderType&SOFMIS_RENDER_MATERIALCOLOR){
//		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,  renderDetail->colorDiffuse.toGLColor());
//		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,renderDetail->colorSpecular.toGLColor());
//		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,renderDetail->colorAmbient.toGLColor());
//		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,renderDetail->shininess);
//
//
//	}	   
//
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableClientState(GL_NORMAL_ARRAY);
//	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
//		if(p_surfaceMesh->isMeshTextured()){
//			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//			if(!shaderEnabled){
//				for(smInt t=0;t<p_surfaceMesh->textureIds.size();t++){
//					glActiveTexture(GL_TEXTURE0+t);
//					smTextureManager::activateTexture(p_surfaceMesh->textureIds[t].textureId);
//				}
//			}
//			else{
//				//for(smInt t=0;t<p_surfaceMesh->textureIds.size();t++){
//					shader->activeGLTextures(p_surfaceMesh->uniqueId);
//				//}
//			
//			}
//
//		}
//
//	}
//
//	if(renderDetail->renderType&SOFMIS_RENDER_COLORMAP)
//		glEnableClientState(GL_COLOR_ARRAY);
//
//	glVertexPointer(3,smGLRealType,0,p_surfaceMesh->vertices);
//	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
//		if(p_surfaceMesh->isMeshTextured())		
//			glTexCoordPointer(2,smGLRealType,0,p_surfaceMesh->texCoord);
//		
//
//
//	}
//
//	glNormalPointer(smGLRealType,0,p_surfaceMesh->vertNormals);
//	if(renderDetail->renderType&SOFMIS_RENDER_FACES){
//		glDrawElements(GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);
//	}
//
//	if((renderDetail->renderType&(SOFMIS_RENDER_VERTICES))){
//		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
//		glDisable(GL_LIGHTING);
//		glDrawElements(GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);
//		glEnable(GL_LIGHTING);
//		//default rendering
//		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
//
//	}
//
//	if(renderDetail->renderType&SOFMIS_RENDER_WIREFRAME){
//		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
//		glLineWidth(renderDetail->lineSize+0.5);
//
//		glPolygonOffset (1.0, 1.0);
//		glDisable(GL_LIGHTING);
//		glDisable(GL_TEXTURE_2D);
//		
//		glColor4fv(renderDetail->wireFrameColor.toGLColor());
//		glDrawElements (GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);	
//		glEnable(GL_LIGHTING);
//		glEnable(GL_TEXTURE_2D);
//		glLineWidth(renderDetail->lineSize);
//		//default rendering
//		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
//
//	}
//	if(renderDetail->renderType&SOFMIS_RENDER_LOCALAXIS){
//		glEnable(GL_LIGHTING);
//	}
//
//	if(renderDetail->renderType&SOFMIS_RENDER_HIGHLIGHTVERTICES){
//		glDisable(GL_LIGHTING);
//		glColor3fv((smGLReal*)&renderDetail->highLightColor);
//		glDrawArrays (GL_POINTS,0,p_surfaceMesh->nbrVertices);
//		glEnable(GL_LIGHTING);
//	}
//
//
//
//	if(renderDetail->renderType&SOFMIS_RENDER_TRANSPARENT){
//		glDisable(GL_BLEND); 
//	}
//
//	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisableClientState(GL_NORMAL_ARRAY);
//	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
//		if(p_surfaceMesh->isMeshTextured()){
//			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//			for(smInt t=0;t<p_surfaceMesh->textureIds.size();t++){
//				glActiveTexture(GL_TEXTURE0+t);			
//				smTextureManager::disableTexture(p_surfaceMesh->textureIds[t].textureId);
//			}
//		}
//	}
//
//	if(renderDetail->renderType&SOFMIS_RENDER_COLORMAP)
//		glDisableClientState(GL_COLOR_ARRAY);
//
//	if(p_drawParam.rendererObject->renderStage!=SMRENDERSTAGE_SHADOWPASS)
//	{
//		 for(smInt i=0;i<renderDetail->shaders.size();i++){
//			if(shaderEnabled){
//				smShader::getShader(renderDetail->shaders[i])->posdraw(p_surfaceMesh);
//				shader->disableShader();
//				break;
//			}
//		}
//	}
//
//	glEnable(GL_LIGHTING);
//	glPointSize(1.0);
//	glLineWidth(1.0);
//}





void smGLRenderer::drawSurfaceMeshTriangles(smMesh *p_surfaceMesh,smRenderDetail *renderDetail, smDrawParam p_drawParam)
{
	static smFloat shadowMatrixGL[16];
	static smVec3<smFloat> origin(0,0,0);
	static smVec3<smFloat> xAxis(1,0,0);
	static smVec3<smFloat> yAxis(0,1,0);
	static smVec3<smFloat> zAxis(0,0,1);
	smShader *shader=NULL;
	smBool shaderEnabled=false;
	smVAO *vao;

	if(renderDetail->renderType&SOFMIS_RENDER_VAO){
		if(renderDetail->VAOs.size()<1)
			return;
		else{ 
			if(renderDetail->VAOEnable[0])
				vao=smVAO::getVAO(renderDetail->VAOs[0]);
			else 
				return;
		}
	
	}
	/* if(p_surfaceMesh->meshFileType==SM_FILETYPE_OBJ){
					glDisableClientState(GL_COLOR_ARRAY);
					glDisableClientState(GL_INDEX_ARRAY);
					glEnableClientState(GL_VERTEX_ARRAY);
			 		 ((smSurfaceMesh *)p_surfaceMesh)->updateOBJStructure();

	 }*/
	 
	if(p_surfaceMesh->renderDetail.renderType&SOFMIS_RENDER_NONE)
		return;
	
	if(p_drawParam.rendererObject->renderStage!=SMRENDERSTAGE_SHADOWPASS)
	{
		for(smInt i=0;i<renderDetail->shaders.size();i++){
			if(renderDetail->shaderEnable[i]){

				shader=smShader::getShader(renderDetail->shaders[i]);
				shader->enableShader();
				
				shader->predraw(p_surfaceMesh);
				shaderEnabled=true;
				//renderDetail->shaders[i].shader->enableShader();
				//renderDetail->shaders[i].shader->preDraw(this);
				break;
			}
		}
	}
	else{
		smGLRenderer::enableDefaultGLRendering();

	}
	
	glDisable(GL_TEXTURE_2D);

	glPointSize(renderDetail->pointSize);
	glLineWidth(renderDetail->lineSize);

	if(p_surfaceMesh->vertTangents!=NULL&&shaderEnabled&&p_surfaceMesh->tangentChannel&&!(renderDetail->renderType&SOFMIS_RENDER_VAO)){
		glVertexAttribPointerARB(shader->tangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, p_surfaceMesh->vertTangents);
		glEnableVertexAttribArrayARB(shader->tangentAttrib);
	}

	if(renderDetail->renderType&SOFMIS_RENDER_TRANSPARENT){
		glEnable (GL_BLEND); 
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc (GL_ONE, GL_ONE);

	}
	if(renderDetail->renderType&SOFMIS_RENDER_MATERIALCOLOR){
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,  renderDetail->colorDiffuse.toGLColor());
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,renderDetail->colorSpecular.toGLColor());
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,renderDetail->colorAmbient.toGLColor());
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,renderDetail->shininess);


	}	   

	if(!(renderDetail->renderType&SOFMIS_RENDER_VAO)){
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
	}
	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
		if(p_surfaceMesh->isMeshTextured()){
			if(!(renderDetail->renderType&SOFMIS_RENDER_VAO))
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if(!shaderEnabled){
				for(smInt t=0;t<p_surfaceMesh->textureIds.size();t++){
					glActiveTexture(GL_TEXTURE0+t);
					smTextureManager::activateTexture(p_surfaceMesh->textureIds[t].textureId);
				}
			}
			else{
				//for(smInt t=0;t<p_surfaceMesh->textureIds.size();t++){
					shader->activeGLTextures(p_surfaceMesh->uniqueId);
				//}
			
			}

		}

	}

	if(renderDetail->renderType&SOFMIS_RENDER_COLORMAP&&!(renderDetail->renderType&SOFMIS_RENDER_VAO))
		glEnableClientState(GL_COLOR_ARRAY);
	if(!(renderDetail->renderType&SOFMIS_RENDER_VAO))
		glVertexPointer(3,smGLRealType,0,p_surfaceMesh->vertices);
	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE&&!(renderDetail->renderType&SOFMIS_RENDER_VAO)){
		if(p_surfaceMesh->isMeshTextured())		
			glTexCoordPointer(2,smGLRealType,0,p_surfaceMesh->texCoord);
		


	}
	if(!(renderDetail->renderType&SOFMIS_RENDER_VAO))
		glNormalPointer(smGLRealType,0,p_surfaceMesh->vertNormals);

	if(renderDetail->renderType&SOFMIS_RENDER_FACES){
		//glPolygonOffset (0.1, 0.1);
		//glDrawElements(GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);
		if(!(renderDetail->renderType&SOFMIS_RENDER_VAO)){
		 /*if(p_surfaceMesh->meshFileType==SM_FILETYPE_OBJ){
			 drawOBJMesh(shader,(smSurfaceMesh *)p_surfaceMesh);
				
		 }
		 else*/
			glDrawElements(GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);
		
		}
		else{
			vao->draw(p_drawParam);
		
		
		}

	}
		
	

	if((renderDetail->renderType&(SOFMIS_RENDER_VERTICES))){
		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
		glDisable(GL_LIGHTING);
		if(!(renderDetail->renderType&SOFMIS_RENDER_VAO))
			glDrawElements(GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);
		glEnable(GL_LIGHTING);
		//default rendering
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	}

	if(renderDetail->renderType&SOFMIS_RENDER_WIREFRAME){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glLineWidth(renderDetail->lineSize+0.5);

		glPolygonOffset (1.0, 1.0);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		
		glColor4fv(renderDetail->wireFrameColor.toGLColor());
		if(!(renderDetail->renderType&SOFMIS_RENDER_VAO))
			glDrawElements (GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);	
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glLineWidth(renderDetail->lineSize);
		//default rendering
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	}
	if(renderDetail->renderType&SOFMIS_RENDER_LOCALAXIS){
		glEnable(GL_LIGHTING);
		//drawArrow((qglviewer::Vec *)&origin,(qglviewer::Vec *)&xAxis,2,12);
		//drawArrow((qglviewer::Vec *)&origin,(qglviewer::Vec *)&yAxis,2,12);
		//drawArrow((qglviewer::Vec *)&origin,(qglviewer::Vec *)&zAxis,2,12);

	}

	//if(renderDetail->renderType&SOFMIS_RENDER_SHADOWS){	
	//	glMatrixMode(GL_MATRIX_MODE);
	//	glPushMatrix();
	//	glDisable(GL_LIGHTING);
	//	shadowMatrix.getMatrixForOpenGL(shadowMatrixGL);
	//	glMultMatrixf((smGLReal*)shadowMatrixGL);
	//	glColor4fv((smGLFloat*)&renderDetail->shadowColor);
	//	glDrawElements (GL_TRIANGLES,p_surfaceMesh->nbrTriangles*3,smGLUIntType,p_surfaceMesh->triangles);		
	//	glEnable(GL_LIGHTING);
	//	glPopMatrix();
	//}


	if(renderDetail->renderType&SOFMIS_RENDER_HIGHLIGHTVERTICES&&!(renderDetail->renderType&SOFMIS_RENDER_VAO)){
		glDisable(GL_LIGHTING);
		glColor3fv((smGLReal*)&renderDetail->highLightColor);
		glDrawArrays (GL_POINTS,0,p_surfaceMesh->nbrVertices);
		glEnable(GL_LIGHTING);
	}



	if(renderDetail->renderType&SOFMIS_RENDER_TRANSPARENT){
		glDisable(GL_BLEND); 
	}

	if(!(renderDetail->renderType&SOFMIS_RENDER_VAO)){
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	if(renderDetail->renderType&SOFMIS_RENDER_TEXTURE){
		if(p_surfaceMesh->isMeshTextured()){
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			for(smInt t=0;t<p_surfaceMesh->textureIds.size();t++){
				glActiveTexture(GL_TEXTURE0+t);			
				smTextureManager::disableTexture(p_surfaceMesh->textureIds[t].textureId);

			}
		}
	}

	if(renderDetail->renderType&SOFMIS_RENDER_COLORMAP&&!(renderDetail->renderType&SOFMIS_RENDER_VAO))
		glDisableClientState(GL_COLOR_ARRAY);


	/*for(smInt i=0;i<renderDetail->shaders.size();i++){
		if(renderDetail->shaders[i].enabled)
			renderDetail->shaders[i].shader->disableShader();
	}*/
	if(p_drawParam.rendererObject->renderStage!=SMRENDERSTAGE_SHADOWPASS)
	{
		 for(smInt i=0;i<renderDetail->shaders.size();i++){
			if(shaderEnabled){
				smShader::getShader(renderDetail->shaders[i])->posdraw(p_surfaceMesh);
				//renderDetail->shaders[i].shader->enableShader();
				//renderDetail->shaders[i].shader->preDraw(this);
				shader->disableShader();
				break;
			}
		}
	}
	
	glEnable(GL_LIGHTING);
	glPointSize(1.0);
	glLineWidth(1.0);


}

void smGLRenderer::drawNormals(smMesh *p_mesh,smColor p_color){
	glDisable(GL_LIGHTING);
	glColor3fv((smGLFloat*)&p_color);
	smVec3f baryCenter;
	smVec3f tmp;

	glBegin(GL_LINES);
	
	for(smInt i=0;i<p_mesh->nbrVertices;i++){
		glVertex3fv((smGLFloat*)&(p_mesh->vertices[i]));
		tmp = p_mesh->vertices[i]+p_mesh->vertNormals[i]*5;
		glVertex3fv((smGLFloat *)&tmp);
	}
	for(smInt i=0;i<p_mesh->nbrTriangles;i++){
		baryCenter=p_mesh->vertices[p_mesh->triangles[i].vert[0]]+p_mesh->vertices[p_mesh->triangles[i].vert[1]]+p_mesh->vertices[p_mesh->triangles[i].vert[2]]	;
		baryCenter=baryCenter/3.0;
		glVertex3fv((smGLFloat*)&(baryCenter));
		tmp = baryCenter+p_mesh->triNormals[i]*5;
		glVertex3fv((smGLFloat *)&tmp);
	}
	glEnd();
	glEnable(GL_LIGHTING);
}

void smGLRenderer::beginTriangles(){
	glBegin(GL_TRIANGLES);
}

void smGLRenderer::drawTriangle(smVec3<smFloat> &p_1,smVec3<smFloat> &p_2,smVec3<smFloat> &p_3){
	glVertex3fv((GLfloat*)&p_1);
	glVertex3fv((GLfloat*)&p_2);
	glVertex3fv((GLfloat*)&p_3);
}

void smGLRenderer::endTriangles(){
	glEnd();
}

void smGLRenderer::draw(smAABB &aabb,smColor p_color){

	glPushAttrib(GL_LIGHTING_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	//glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );

	glLineWidth(1.0);
	glColor3fv(p_color.toGLColor());		
	glBegin(GL_LINES);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMin.z);
		glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMin.z);

		glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMin.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMin.z);

		glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y,aabb.aabbMin.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y,aabb.aabbMax.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMax.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMax.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y,aabb.aabbMax.z);
		glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMax.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMax.z);
		glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMin.z);

		glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMax.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMax.z);

		glVertex3f(aabb.aabbMin.x,aabb.aabbMax.y, aabb.aabbMax.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMin.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMax.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMax.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMax.z);
		glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMin.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMin.z);
		glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMin.z);

		glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMin.z);
		glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMin.z);
	glEnd();
	glLineWidth(1.0);
	glEnable(GL_LIGHTING);
	//glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
	glPopAttrib();

}
  /*void smGLRenderer::drawCoordSystem(smVec3<smFloat> p_pos,smVec3<smFloat> xDirection,smVec3<smFloat> yDirection,smVec3<smFloat> zDirection,smFloat p_scale,smInt p_subDivisions, smFloat p_length){
	static GLUquadric* quadric = gluNewQuadric();
	smMatrix44<smFloat> mat;
	smFloat radius=5.0*p_scale;
	smInt nbSubdivisions=p_subDivisions;
	smFloat length=p_length;
	smFloat matGL[16];
	
	glPushMatrix();

	glTranslatef(p_pos.x,p_pos.y,p_pos.z);
	mat.setColumn(xDirection,0);
	mat.setColumn(yDirection,1);
	mat.setColumn(zDirection,2);
	mat.getMatrixForOpenGL(matGL);
	glMultMatrixf(matGL);

	


	if (radius < 0.0)
		radius = 0.05 * length;

	const float head = 2.5*(radius / length) + 0.1;
	const float coneRadiusCoef = 4.0 - 5.0 * head;
	

	gluCylinder(quadric, radius, radius, length * (1.0 - head/coneRadiusCoef), nbSubdivisions, 1);
	glTranslatef(0.0, 0.0, length * (1.0 - head));
	gluCylinder(quadric, coneRadiusCoef * radius, 0.0, head * length, nbSubdivisions, 1);
	glTranslatef(0.0, 0.0, -length * (1.0 - head));
	glPopMatrix();

 }*/
void smGLRenderer::drawCoordSystem(smViewer *viewer,QString p_name,smVec3<smFloat> p_pos,smVec3<smFloat> dirX,smVec3<smFloat> dirY,smVec3<smFloat> dirZ){	  //static qglviewer::Camera cam;
	qglviewer::Vec  vec;
	smFloat p_scale=5.0;

	dirX=dirX*p_scale+p_pos;
	dirY=dirY*p_scale+p_pos;
	dirZ=dirZ*p_scale+p_pos;
	glPushAttrib(GL_TEXTURE_BIT);
	vec=viewer->camera()->projectedCoordinatesOf(qglviewer::Vec(p_pos.x,p_pos.y,p_pos.z));
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	viewer->drawText(vec.x,vec.y,p_name);

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,smColor::colorRed.toGLColor());
	viewer->drawArrow(qglviewer::Vec(p_pos.x,p_pos.y,p_pos.z), qglviewer::Vec(dirX.x,dirX.y,dirX.z),0.1);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,smColor::colorGreen.toGLColor());
	viewer->drawArrow(qglviewer::Vec(p_pos.x,p_pos.y,p_pos.z), qglviewer::Vec(dirY.x,dirY.y,dirY.z),0.1);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,smColor::colorBlue.toGLColor());
	viewer->drawArrow(qglviewer::Vec(p_pos.x,p_pos.y,p_pos.z), qglviewer::Vec(dirZ.x,dirZ.y,dirZ.z),0.1);
	glPopAttrib();	

}



void smGLRenderer::draw(smPlane &p_plane,smFloat p_scale, smColor p_color)
{
	smQuat<smFloat> rot;
	smFloat angle;
	smVec3f axisOfRot;
	smVec3f defaultDir(0,0,1);
	smVec3f planePoints[4]={smVec3f(-p_scale,p_scale,0),
	                        smVec3f(-p_scale,-p_scale,0),
	                        smVec3f(p_scale, -p_scale,0),
	                        smVec3f(p_scale,p_scale,0)};
	smVec3f tmp;

	angle = acos(defaultDir.dot(p_plane.unitNormal));
	axisOfRot = p_plane.unitNormal.cross(defaultDir);
	axisOfRot.normalize();

	rot.fromAxisAngle(axisOfRot,-angle);

	glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
		glColor3fv(p_color.toGLColor());
		tmp = rot.rotate(planePoints[0])+p_plane.pos;
		glVertex3fv((smGLFloat*)&tmp);
		tmp = rot.rotate(planePoints[1])+p_plane.pos;
		glVertex3fv((smGLFloat*)&tmp);
		tmp = rot.rotate(planePoints[2])+p_plane.pos;
		glVertex3fv((smGLFloat*)&tmp);
		tmp = rot.rotate(planePoints[3])+p_plane.pos;
		glVertex3fv((smGLFloat*)&tmp);
	glEnd();
	glEnable(GL_LIGHTING);
}



void smGLRenderer::enableDefaultGLRendering()
{
	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	//glDisable(GL_GEOMETRY_SHADER_ARB);
	glUseProgramObjectARB(0);
}
