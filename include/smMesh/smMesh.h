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

\author:    Module by Sreekanth A V


\version    1.0
\date       05/2009
\bug	    None yet
\brief	    This Module is for generic Mesh class.

*****************************************************
*/


#ifndef SMMESH_H
#define SMMESH_H

#include <QVector>
#include <QAtomicInt>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smCore/smTextureManager.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix33.h"
#include "smCore/smMemoryBlock.h"
#include "smCollision/smCollisionConfig.h"
#include "smCore/smGeometry.h"

#define SOFMIS_MESH_AABBSKINFACTOR	0.1			  ///Bounding box skin value
#define SOFMIS_MESH_RESERVEDMAXEDGES		6000  ///this value is initially allocated buffer size for thge edges

struct smTexCoord;
struct smTriangle;
struct smTetrahedra;
struct smEdge;
//struct smAABB;


enum smMeshType
{
	SMMESH_DEFORMABLE,
	SMMESH_DEFORMABLECUTABLE,
	SMMESH_RIGIDCUTABLE,
	SMMESH_RIGID      
};

enum smMeshFileType
{
	SM_FILETYPE_NONE,
	SM_FILETYPE_OBJ,
	SM_FILETYPE_3DS,
	SM_FILETYPE_VOLUME,
};

class smShader;

//struct smShaderAttachment{
//	~smShaderAttachment(){
//	
//	}
//	smShaderAttachment(){
//		shader=NULL;
//		enabled=true;
//	
//	}
//   smShader *shader;
//   smBool enabled;
//   
//   inline smBool operator ==(smShaderAttachment &p_param);
//   inline smBool operator ==(QString &p_param);
//};


struct smTextureAttachment{
 smTextureAttachment(){
	    //textureShaderAttachment=textureId=-1;
		//shaderAttached=false;
  }
  smInt textureId;
  //vector<smChar*> p_attached
  //vector<smGLInt> textureShaderAttachment;
  //vector<smUnifiedID> shaderIDs;

};

class smBaseMesh:public smCoreClass{
public:
	
	smCollisionGroup collisionGroup;

	smGLInt renderingID;
	smErrorLog *log;
	smVec3<smFloat> *vertices;   
	smVec3<smFloat> * origVerts;
	//smUnifiedID meshId;   
	smInt  nbrVertices;
	smAABB aabb;
	smBool isTextureCoordAvailable;
	///texture
	smTexCoord *texCoord;
	vector<smTextureAttachment> textureIds;
	smBaseMesh();
	inline smBool isMeshTextured(){
		return isTextureCoordAvailable;
	}
	//void assignShaderTexture(smChar *p_textureReferenceName, smUnifiedID p_shaderID,smChar *p_textureParamName){
	//	smInt tempTextureId;
	//	smTextureManager::findTextureId(p_textureReferenceName,tempTextureId);
	//	for(smInt i=0;i<textureIds.size();i++){
	//		if(textureIds[i].textureId==tempTextureId){
	//			//textureIds[i].textureShaderAttachment.p_shaderParamImage;
	//			textureIds[i].
	//			
	//			textureIds[i].shaderIDs.push_back(shaderID);


	//		}
	//		
	//	}
	//
	//
	//}
	void assignTexture(smInt p_textureId){
		smTextureAttachment attachment;
		attachment.textureId=p_textureId;
		if(p_textureId>0)
			textureIds.push_back(attachment);


	}
	void assignTexture(smChar *p_referenceName){
		smInt textureId;
		smTextureAttachment attachment;
		if(smTextureManager::findTextureId(p_referenceName,textureId)==SOFMIS_TEXTURE_OK){
			attachment.textureId=textureId;
			textureIds.push_back(attachment);
		}


	}

	void updateOriginalVertsWithCurrent();

  

};



///brief: this is a generic Mesh class from which surface and volume meshes are inherited
///Note: this class cannot exist on its own
class smMesh:public smBaseMesh{
protected:

public:
	smInt  nbrTriangles;
	smTriangle *triangles;

	smTexCoord *texCoordForTrianglesOBJ;//tansel for OBJ
	int	nbrTexCoordForTrainglesOBJ;//tansel for OBJ

	///texture
	//smTexCoord *texCoord;

	smVec3<smFloat> *triNormals;
	smVec3<smFloat> *vertNormals;


	smVec3<smFloat> *triTangents;
	smVec3<smFloat> *vertTangents;	
	smBool tangentChannel;

	///triangle neighbors for the vertices

	vector< vector<smInt> > vertTriNeighbors;
	vector< vector<smInt> > vertVertNeighbors;
	vector<smEdge> edges;


	///AABBB of the mesh.
	///This value is allocated and computed by only collision detection module
	///Therefore it is initally NULL
	smAABB *triAABBs;

	smMeshType meshType;

	smMeshFileType meshFileType;

	static QAtomicInt meshIdCounter;

	///get unique Id
	/*static smInt getNewMeshId(){
	smInt ret;
	ret=meshIdCounter.fetchAndAddOrdered(1);
	return ret;
	}*/

public:
	smMesh();
	virtual ~smMesh();

	void getVertexNeighbors();
	void getTriangleNeighbors();

	smBool initVertexArrays(smInt nbr);
	smBool initTriangleArrays(smInt nbr);

	void initVertexNeighbors();
	void allocateAABBTris();

	 smVec3<smFloat> calculateTriangleNormal(smInt triNbr);

	void updateTriangleNormals();
	void updateVertexNormals();
	void upadateAABB();
	void updateTriangleAABB();

	void calcTriangleTangents();
	void calculateTangent(smVec3<smFloat>& p1,smVec3<smFloat>& p2,smVec3<smFloat>& p3,smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3<smFloat>& t);
	void calculateTangent_test(smVec3<smFloat>& p1,smVec3<smFloat>& p2,smVec3<smFloat>& p3,smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3<smFloat>& t);
	void calcNeighborsVertices();
	void calcEdges();


	void translate(smFloat,smFloat,smFloat);
	void translate(smVec3<smFloat> p_offset);
	void scale(smVec3<smFloat> p_scaleFactors);

	void rotate(smMatrix33<smFloat> p_rot);
	void checkCorrectWinding();



	smMeshType getMeshType(){return meshType;};




	virtual smBool loadMesh(smChar *fileName,smMeshFileType fileType)=0;

	virtual void draw(smDrawParam p_params);

};


struct smTexCoord
{
	smFloat u,v;
};

struct smTriangle
{
	smUInt vert[3];

};

struct smTetrahedra
{
	smInt vert[4];
};

struct smEdge
{
	smUInt vert[2];
	//smUInt tri[2];	
};


class smLineMesh:public smBaseMesh{
public: 

	smAABB *edgeAABBs;
	smEdge *edges;
	smInt nbrEdges;


	~smLineMesh(){
		delete[]vertices;
		delete[]origVerts;
		delete[]edgeAABBs;
		delete[]texCoord;
		delete[]edges;
	}

	smLineMesh(smInt p_nbrVertices):smBaseMesh()
	{
		

		nbrVertices=p_nbrVertices;
		vertices=new smVec3<smFloat>[nbrVertices];
		origVerts=new smVec3<smFloat>[nbrVertices];
		edgeAABBs=new smAABB[nbrVertices-1];
		texCoord=new smTexCoord[nbrVertices];
		edges=new smEdge[nbrVertices-1];
		nbrEdges=nbrVertices-1;
		isTextureCoordAvailable=false;
		createAutoEdges();


	}


	smLineMesh(smInt p_nbrVertices, smBool autoEdge):smBaseMesh()
	{


		nbrVertices=p_nbrVertices;
		vertices=new smVec3<smFloat>[nbrVertices];
		origVerts=new smVec3<smFloat>[nbrVertices];
		
		texCoord=new smTexCoord[nbrVertices];

		/// Edge AABB should be assigned by the instance
		edgeAABBs= NULL; 
		/// Edges should be assigned by the instance
		edges= NULL; 

		/// Number of edges should be assigned by the instance
		nbrEdges=0;
		isTextureCoordAvailable=false;

		if(autoEdge)
			createAutoEdges();




	}



	void createAutoEdges(){
		//smEdge edge;
		for(smInt i=0;i<nbrEdges;i++){
			edges[i].vert[0]=i;
			edges[i].vert[1]=i+1;
		}
		//edges.push_back(edge);


	}

	virtual void createCustomEdges(){};

	inline void updateAABB(){
		smAABB tempAABB;
		smVec3<smFloat> minOffset(-2.0,-2.0,-2.0);
		smVec3<smFloat> maxOffset(1.0,1.0,1.0);
		smVec3<smFloat> minEdgeOffset(-0.1,-0.1,-0.1);
		smVec3<smFloat> maxEdgeOffset(0.1,0.1,0.1);

		tempAABB.aabbMin.x=FLT_MAX;
		tempAABB.aabbMin.y=FLT_MAX;
		tempAABB.aabbMin.z=FLT_MAX;

		tempAABB.aabbMax.x=-FLT_MAX;
		tempAABB.aabbMax.y=-FLT_MAX;
		tempAABB.aabbMax.z=-FLT_MAX;

		for(smInt i=0;i<nbrEdges;i++)
		{
			//min
			edgeAABBs[i].aabbMin.x = SOFMIS_MIN(vertices[edges[i].vert[0]].x,vertices[edges[i].vert[1]].x );
			edgeAABBs[i].aabbMin.y = SOFMIS_MIN(vertices[edges[i].vert[0]].y,vertices[edges[i].vert[1]].y );
			edgeAABBs[i].aabbMin.z = SOFMIS_MIN(vertices[edges[i].vert[0]].z,vertices[edges[i].vert[1]].z );
			edgeAABBs[i].aabbMin+=minEdgeOffset;
			tempAABB.aabbMin.x=SOFMIS_MIN(tempAABB.aabbMin.x,edgeAABBs[i].aabbMin.x);
			tempAABB.aabbMin.y=SOFMIS_MIN(tempAABB.aabbMin.y,edgeAABBs[i].aabbMin.y);
			tempAABB.aabbMin.z=SOFMIS_MIN(tempAABB.aabbMin.z,edgeAABBs[i].aabbMin.z);

			//max
			edgeAABBs[i].aabbMax.x = SOFMIS_MAX(vertices[edges[i].vert[0]].x,vertices[edges[i].vert[1]].x );
			edgeAABBs[i].aabbMax.y = SOFMIS_MAX(vertices[edges[i].vert[0]].y,vertices[edges[i].vert[1]].y );
			edgeAABBs[i].aabbMax.z = SOFMIS_MAX(vertices[edges[i].vert[0]].z,vertices[edges[i].vert[1]].z );
			edgeAABBs[i].aabbMax+=maxEdgeOffset;
			tempAABB.aabbMax.x=SOFMIS_MAX(tempAABB.aabbMax.x,edgeAABBs[i].aabbMax.x);
			tempAABB.aabbMax.y=SOFMIS_MAX(tempAABB.aabbMax.y,edgeAABBs[i].aabbMax.y);
			tempAABB.aabbMax.z=SOFMIS_MAX(tempAABB.aabbMax.z,edgeAABBs[i].aabbMax.z);
		}

		tempAABB.aabbMin+=minOffset;
		tempAABB.aabbMax+=maxOffset;
		aabb=tempAABB;
	}

	void translate(smFloat p_offsetX,smFloat p_offsetY,smFloat p_offsetZ)
	{		
		for(smInt i=0; i<nbrVertices; i++)
		{
			vertices[i].x=vertices[i].x+p_offsetX;
			vertices[i].y=vertices[i].y+p_offsetY;
			vertices[i].z=vertices[i].z+p_offsetZ;
		}
		updateAABB();
	}
	void translate(smVec3<smFloat> p_offset)
	{		
		for(smInt i=0; i<nbrVertices; i++)
		{
			vertices[i]=vertices[i]+p_offset;
			origVerts[i]=origVerts[i]+p_offset;

		}
		updateAABB();
	}

	void scale(smVec3<smFloat> p_scaleFactors)
	{		
		for(smInt i=0; i<nbrVertices; i++)
		{
			vertices[i].x=vertices[i].x*p_scaleFactors.x;
			vertices[i].y=vertices[i].y*p_scaleFactors.y;
			vertices[i].z=vertices[i].z*p_scaleFactors.z;

			origVerts[i].x=origVerts[i].x*p_scaleFactors.x;
			origVerts[i].y=origVerts[i].y*p_scaleFactors.y;
			origVerts[i].z=origVerts[i].z*p_scaleFactors.z;
		}
		updateAABB();
	}

	void rotate(smMatrix33<smFloat> p_rot)
	{		
		for(smInt i=0; i<nbrVertices; i++)
		{
			vertices[i]=p_rot*vertices[i];
			origVerts[i]=p_rot*origVerts[i];
		}

		updateAABB();
	}

	inline smBool isMeshTextured(){
		return isTextureCoordAvailable;
	}
	void draw(smDrawParam p_params);
};


#endif
