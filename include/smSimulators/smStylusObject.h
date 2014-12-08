/*
****************************************************
                  SIMMEDTK LICENSE
****************************************************
*/

#ifndef SMSTYLUSOBJECT_H
#define SMSTYLUSOBJECT_H

#include <QVector>
#include <QStack>

#include "smCore/smConfig.h"
#include "smMesh/smMesh.h"
#include "smCore/smSceneObject.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMath.h"
#include "smExternal/tree.hh"

template<typename T> class smCollisionModel;
template<typename smSurfaceTreeCell> class smSurfaceTree;
struct smOctreeCell;

/// \brief !!
struct smMeshContainer{
public:
	QString name;

	/// \brief constructor
	smMeshContainer(QString p_name=""){
		name=p_name;
		offsetRotX=0.0;
		offsetRotY=0.0;
		offsetRotZ=0.0;
		preOffsetPos.setValue(0,0,0);
		posOffsetPos.setValue(0,0,0);
		mesh=NULL;
		colModel=NULL;
	}

	/// \brief constructor
	smMeshContainer(QString p_name,smMesh *p_mesh,smVec3<smFloat> p_prePos,smVec3<smFloat> p_posPos,smFloat p_offsetRotX,smFloat p_offsetRotY,smFloat p_offsetRotZ){
		offsetRotX=p_offsetRotX;
		offsetRotY=p_offsetRotY;
		offsetRotZ=p_offsetRotZ;
		preOffsetPos=p_prePos;
		posOffsetPos=p_posPos;
		name=p_name;
		colModel=NULL;
	}

	smFloat offsetRotX; ///< offset in rotation in x-direction
	smFloat offsetRotY; ///< offset in rotation in y-direction
	smFloat offsetRotZ; ///< offset in rotation in z-direction
	smVec3<smDouble> preOffsetPos; ///< !!
	smVec3<smDouble> posOffsetPos; ///< !!
	smMatrix44<smDouble> accumulatedMatrix; ///< !!
	smMatrix44<smDouble> accumulatedDeviceMatrix; ///< !!

	smMatrix44<smDouble> currentMatrix; ///< !!
	smMatrix44<smDouble> currentViewerMatrix; ///< !!
	smMatrix44<smDouble> currentDeviceMatrix; ///< !!
	smMatrix44<smDouble> tempCurrentMatrix; ///< !!
	smMatrix44<smDouble> tempCurrentDeviceMatrix; ///< !!
	smMesh * mesh; ///< mesh
	smSurfaceTree<smOctreeCell> *colModel; ///< octree of surface

	/// \brief !!
	inline void computeCurrentMatrix(){
		smMatrix33<smDouble> matX,matY,matZ;
		smMatrix33<smDouble> res;
		smMatrix44<smDouble> trans,trans1;
		smMatrix44<smDouble> temp;

		trans.setTranslation(preOffsetPos.x,preOffsetPos.y,preOffsetPos.z);
		matX.rotAroundX(SM_PI_TWO*offsetRotX);
		matY.rotAroundY(SM_PI_TWO*offsetRotY);
		matZ.rotAroundZ(SM_PI_TWO*offsetRotZ);
		trans1.setTranslation(posOffsetPos.x,posOffsetPos.y,posOffsetPos.z);

		res=matX*matY*matZ;
		tempCurrentMatrix=accumulatedMatrix*trans*res*trans1;
		tempCurrentDeviceMatrix=accumulatedDeviceMatrix*trans*res*trans1;
	}
};

/// \brief points on the stylus
struct smStylusPoints{

	/// \brief constructor
	smStylusPoints(){
		point.setValue(0,0,0);
		container=NULL;
	}

	smVec3 <smFloat> point; ///< co-ordinates of points on stylus
	smMeshContainer *container; ///< !!
};

/// \brief stylus object of the scene (typically used for laparascopic VR simulations)
class smStylusSceneObject:public smSceneObject{

public:
	smVec3 <smDouble> pos; ///< position of stylus
	smVec3 <smDouble> vel; ///< velocity of stylus
	smMatrix33 <smDouble> rot; ///< rotation of stylus
	smMatrix44 <smDouble> transRot; ///< !! translation and rotation matrix of stylus
	smMatrix44 <smDouble> transRotDevice; ///< translation and rotation matrix of devide controlling the stylus
	smBool toolEnabled; ///< !!

	/// \brief constructor
	smStylusSceneObject(smErrorLog *p_log=NULL);
	
	/// \brief !!
	virtual void serialize(void *p_memoryBlock){
	}

	/// \brief !!
	virtual void unSerialize(void *p_memoryBlock){
	}

	/// \brief handle the events such as button presses related to stylus
	virtual void handleEvent(smEvent *p_event){};
};

/// \brief !!
class smStylusRigidSceneObject:public smStylusSceneObject,public smEventHandler{
	QHash<QString,tree<smMeshContainer*>::iterator> indexIterators;
public:
	tree<smMeshContainer*> meshes; ///< meshes representing the stylus
	tree<smMeshContainer*>::iterator rootIterator; ///< !!
	volatile smBool updateViewerMatrixEnabled; ///< !!

	/// \brief to show the device tool..It is for debugging god object
	smBool enableDeviceManipulatedTool;

	/// \brief enabling post traverse callback enabled for each node.
	smBool posTraverseCallbackEnabled;

	/// \brief post traverse callback for each node
	virtual void posTraverseCallBack(smMeshContainer &p_container);

	/// \brief Post Traverse callback for the entire object.
	virtual void posTraverseCallBack(){
	}

	smBool posCallBackEnabledForEntireObject; ///< !!

	/// \brief !!
	smStylusRigidSceneObject(smErrorLog *p_log=NULL);

	/// \brief !!
	tree<smMeshContainer*>::iterator &addMeshContainer(smMeshContainer *p_meshContainer){
		tree<smMeshContainer*>::iterator iter;
		if(meshes.size()>1)
			iter=meshes.append_child(rootIterator,p_meshContainer);

		else
			iter=meshes.insert(rootIterator,p_meshContainer);
		indexIterators.insert(p_meshContainer->name,iter);
		return iter;
	}

	/// \brief !!
	smBool addMeshContainer(QString p_ParentName,smMeshContainer *p_meshContainer){
		tree<smMeshContainer*>::iterator iter;
		if(p_ParentName.size()>0){
			if(indexIterators.contains(p_ParentName)){
				iter=indexIterators[p_ParentName];
				meshes.append_child(iter,p_meshContainer);
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}

	/// \brief !!
	tree<smMeshContainer*>::iterator addMeshContainer(tree<smMeshContainer*>::iterator p_iterator,smMeshContainer *p_meshContainer){
		return meshes.insert(p_iterator,p_meshContainer);
	}

	/// \brief !!
	smMeshContainer *getMeshContainer(QString p_string) const;

	virtual void handleEvent(smEvent *p_event){};

	/// \brief !! 
	smSceneObject *clone(){
		smStylusRigidSceneObject *ret=new smStylusRigidSceneObject();
		return ret;
	}

	/// \brief !!
	virtual void initDraw(smDrawParam p_params);

	/// \brief !!
	virtual void draw(smDrawParam p_params);
};

/// \brief !!
class smStylusDeformableSceneObject:public smStylusSceneObject{
	smStylusDeformableSceneObject(smErrorLog *p_log=NULL);
};

#endif
