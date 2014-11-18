#include "smSimulators/smStylusObject.h"
#include "smCollision/smCollisionModel.h"
#include "smCollision/smCollisionModel.hpp"
#include "smMesh/smMesh.h"






//
//void smMeshContainer::computeCurrentMatrix(){
//	smMatrix33<smFloat> matX,matY,matZ;
//	smMatrix33<smFloat> res;
//	smMatrix44<smFloat> trans,trans1;
//	
//	trans.setTranslation(preOffsetPos.x,preOffsetPos.y,preOffsetPos.z);
//	matX.rotAroundX(SM_PI_TWO*offsetRotX);
//	matY.rotAroundY(SM_PI_TWO*offsetRotY);
//	matZ.rotAroundZ(SM_PI_TWO*offsetRotZ);
//	trans1.setTranslation(posOffsetPos.x,posOffsetPos.y,posOffsetPos.z);
//
//	res=matX*matY*matZ;
//	currentMatrix=accumulatedMatrix*trans*res*trans1;
//
//
//
//
//
//}
//void smStylusRigidSceneObject::draw(smDrawParam p_params){
//		smDouble matrix[16];
//		smDouble matrixTransRot[16];
//		smMatrix44<smFloat> mat;
//
//		glMatrixMode(GL_MODELVIEW);
//		glPushMatrix();
//		transRot.getMatrixForOpenGL(matrixTransRot);
//		glMultMatrixd(matrixTransRot);
//		tree<smMeshContainer>::pre_order_iterator iter= meshes.begin();
//		
//		
//		
//		
//		//update the Root node and render it first
//		iter.node->data.computeCurrentMatrix(); 
//		iter.node->data.currentMatrix.getMatrixForOpenGL(matrix);
//		glPushMatrix();
//			
//		  glCallList(iter->mesh->renderingID);
//		glPopMatrix();
//		iter++;
//		while(iter!=meshes.end()){
//			glPushMatrix();
//				///the parent nodes matrix should be accumulated
//				iter.node->data.accumulatedMatrix=iter.node->parent->data.currentMatrix;
//				iter.node->data.computeCurrentMatrix(); 
//				iter.node->data.currentMatrix.getMatrixForOpenGL(matrix);
//				glMultMatrixd(matrix);
//				glCallList(iter->mesh->renderingID);
//				
//			glPopMatrix();
//			iter++;
//		}
//								
//	glPopMatrix();
//}

void smStylusRigidSceneObject::draw(smDrawParam p_params){
		smDouble matrix[16];
		smDouble matrixTransRot[16];
		
		smMatrix44<smDouble> temp;

		#pragma unroll 
		for(smInt i=0;i<2;i++){
			

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			
			
			tree<smMeshContainer*>::pre_order_iterator iter= meshes.begin();
			
				
			glPushMatrix();
				//if(i==1&&enableDeviceManipulatedTool){
				if(i==2&&enableDeviceManipulatedTool){
					temp=iter.node->data->currentDeviceMatrix;
			
				}
				else
					temp=iter.node->data->currentViewerMatrix;
				temp.getMatrixForOpenGL(matrix);
				glMultMatrixd(matrix);
				glCallList(iter.node->data->mesh->renderingID);
				//glutSolidCone(5.0,15,15,15);
				//smVec3d pos=temp.getColumn(3);
				//cout<<pos.x<<","<<pos.y<<","<<pos.z<<endl; //For debugging purpose..
			glPopMatrix();
			
			iter++;
			while(iter!=meshes.end()){
				glPushMatrix();
					
					//if(i==1&&enableDeviceManipulatedTool){
					if(i==2&&enableDeviceManipulatedTool){
						temp=iter.node->data->currentDeviceMatrix;
			
					}
					else
						temp=iter.node->data->currentViewerMatrix;//.getMatrixForOpenGL(matrix);
					temp.getMatrixForOpenGL(matrix);
					glMultMatrixd(matrix);
					glCallList(iter.node->data->mesh->renderingID);
					
				glPopMatrix();
				iter++;
			}
									
			glPopMatrix();
		}
}





void smStylusRigidSceneObject::initDraw(smDrawParam p_params)
{				smChar errorText[500];
				p_params.caller=this;
				tree<smMeshContainer*>::pre_order_iterator iter= meshes.begin();
				smGLInt newList= glGenLists(meshes.size());
				smGLUtils::queryGLError(errorText) ;
				
				smInt listCounter=0;
				while(iter!=meshes.end()){
					glNewList(newList+listCounter,GL_COMPILE);
					iter.node->data->mesh->draw(p_params);
					glEndList();
					iter.node->data->mesh->renderingID=(newList+listCounter);
					listCounter++;
					iter++;
		}	

}
smStylusSceneObject::smStylusSceneObject(smErrorLog *p_log){
	type=SOFMIS_SMSTYLUSSCENEOBJECT;
	toolEnabled=true;
	

}

smStylusRigidSceneObject::smStylusRigidSceneObject(smErrorLog *p_log){
	type=SOFMIS_SMSTYLUSRIGIDSCENEOBJECT;
	updateViewerMatrixEnabled=true;
	rootIterator=meshes.begin();
	posCallBackEnabledForEntireObject=false;
	enableDeviceManipulatedTool=false;


}


smStylusDeformableSceneObject::smStylusDeformableSceneObject(smErrorLog *p_log){
	type=SOFMIS_SMSTYLUSDEFORMABLESCENEOBJECT;

}


smMeshContainer *smStylusRigidSceneObject::getMeshContainer(QString p_string) const
{
	 tree<smMeshContainer*>::iterator iter= meshes.begin_leaf();
	 
	 while(iter!=meshes.end_leaf()){
		 if(iter.node->data->name==p_string){
			return (*iter);
			
		 }
		 iter++;
	 
	 }




}

void smStylusRigidSceneObject::posTraverseCallBack(smMeshContainer &p_container){
	if(p_container.colModel!=NULL){
		p_container.colModel->transRot=p_container.currentMatrix;
		p_container.colModel->translateRot();
	}


}
//




