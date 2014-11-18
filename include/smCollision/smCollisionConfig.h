/*
****************************************************
SOFMIS LICENSE

****************************************************

\author:    <http:\\acor.rpi.edu>
SOFMIS TEAM IN ALPHABATIC ORDER
Anderson Maciel, Ph.D.
Ganesh Sankaranarayanan, Ph.D.
Sreekanth A Venkata
Suvranu De, Ph.D.
Tansel Halic
Zhonghua Lu

\author:    Module by Tansel Halic


\version    1.0
\date       05/2009
\bug	    None yet
\brief	    This class has collision detection data structures



*****************************************************
*/


#ifndef SMCOLLISIONCONFIG_H
#define SMCOLLISIONCONFIG_H
#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smCore/smCoreClass.h"


struct smCellTriangle{
	smUInt primID;
	smUnifiedID meshID;
	smVec3<smFloat> vert[3];
	inline  bool operator ==(unsigned int p_ID){
		return (primID==p_ID?true:false);
	}
	inline  bool operator ==(smCellTriangle &p_tri){
		return (p_tri.primID==primID?true:false);
	}

	friend ostream &operator<<( ostream &out, smCellTriangle &tri ) {
		    
	   out<<tri.primID;
	   return out;
	}
};
struct smCellLine{
	smUInt primID;   ///Edge id
	smUnifiedID meshID;	 /// smMeshLine id
	smVec3<smFloat> vert[2]; ///Vertices 
	inline  bool operator ==(unsigned int p_ID){
		return (primID==p_ID?true:false);
	}
	inline  bool operator ==(smCellLine &p_line){

		return (p_line.primID==primID?true:false);
	}

   friend ostream &operator<<( ostream &out, smCellLine &p ) {
		    
	   out<<p.primID;
		return out;
	}
};


struct smCellModel{
	smUInt primID;   ///Model Prim id
	smUnifiedID meshID;	 /// smMeshLine id
	smVec3<smFloat> center; ///Vertices
	smFloat radius;
	
	inline  bool operator ==(unsigned int p_ID){
		return (primID==p_ID?true:false);
	}
	inline  bool operator ==(smCellModel &p_model){
		return (p_model.primID==primID?true:false);
	}

   friend ostream &operator<<( ostream &out, smCellModel &p ) {
		    
	   out<<p.primID;
		return out;
	}


};

struct smCellPoint{
	smUInt primID;   ///Model Prim id
	smUnifiedID meshID;	 /// smMeshLine id
	smVec3<smFloat> vert; ///Vertices
	
	inline  bool operator ==(unsigned int p_ID){
		return (primID==p_ID?true:false);
	}
	inline  bool operator ==(smCellPoint &p_point){
		return (p_point.primID==primID?true:false);
	}

   friend ostream &operator<<( ostream &out, smCellPoint &p ) {
		    
	   out<<p.primID;
		return out;
	}


};

struct smCollidedTriangles{
	   smCellTriangle tri1;
	   smCellTriangle tri2;
	   smVec3<smFloat> proj1,proj2;
 	   smShort point1,point2;
	
};

struct smCollidedLineTris{
	smCellTriangle tri;
	smCellLine line;
	smVec3<smFloat> intersection;
};

struct smCollidedModelPoints{
	smCellModel model;
	smCellPoint point;
	smFloat penetration;
};



class smCollisionGroup{
	
protected:
public:
	smUInt64  groupId;
	smUInt64  groupMask;

	smCollisionGroup(){
		groupId=1;
		groupMask=1;
	}
	void setGroup(smUInt p_groupId){
		groupId=1<<p_groupId;
		groupMask=groupId;

	}
	void permitCollision(smCollisionGroup &p_group){
			groupMask=groupMask|(p_group.groupId);
			p_group.groupMask=p_group.groupMask|(groupId);
			cout<<"Mask:"<<groupMask<<endl;
			cout<<"GroupId:"<<groupId<<endl;

	}
	void forbidCollision(smCollisionGroup &p_group){
			groupMask=groupMask&(~p_group.groupId);
			p_group.groupMask=p_group.groupMask&(~groupId);

	}
	

	///if the group is different then the function checks the group mask..
	//if they're in the same group collision enabled
	inline smBool isCollisionPermitted(smCollisionGroup& p_group){

		 //return ( (groupId&p_group.groupId)==0?( (groupMask&p_group.groupMask)==0? false:true):false);
		return ( (groupId&p_group.groupId)==0?((groupId&p_group.groupMask)==0? false:true):true);
	}
	



};




#endif