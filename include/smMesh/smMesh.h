/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
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

#define SIMMEDTK_MESH_AABBSKINFACTOR 0.1  ///Bounding box skin value
#define SIMMEDTK_MESH_RESERVEDMAXEDGES 6000  ///this value is initially allocated buffer size for thge edges

struct smTexCoord;
struct smTriangle;
struct smTetrahedra;
struct smEdge;

/// \brief designates what purpose/scenario the mesh is used for
enum smMeshType{
	SMMESH_DEFORMABLE,
	SMMESH_DEFORMABLECUTABLE,
	SMMESH_RIGIDCUTABLE,
	SMMESH_RIGID
};

/// \brief designates input mesh file type
enum smMeshFileType{
	SM_FILETYPE_NONE,
	SM_FILETYPE_OBJ,
	SM_FILETYPE_3DS,
	SM_FILETYPE_VOLUME,
};

class smShader;

/// \brief !!
struct smTextureAttachment{
	smTextureAttachment(){
	}
	smInt textureId;
};

/// \brief base class for the mesh
class smBaseMesh:public smCoreClass{

public:
	smCollisionGroup collisionGroup; ///< !!
	smGLInt renderingID; ///< !!
	smErrorLog *log; ///< record the log
	smVec3<smFloat> *vertices; ///< vertices co-ordinate data at time t
	smVec3<smFloat> * origVerts; ///< vertices co-ordinate data at time t=0
	smInt  nbrVertices; ///< number of vertices 
	smAABB aabb; ///< Axis aligned bounding box
	smBool isTextureCoordAvailable; ///< true if the texture co-ordinate is available
	smTexCoord *texCoord; ///< texture co-ordinates
	vector<smTextureAttachment> textureIds; ///< !!

	/// \brief constructor
	smBaseMesh();

	/// \brief query if the mesh has textures available for rendering
	inline smBool isMeshTextured(){
		return isTextureCoordAvailable;
	}

	/// \brief assign the texture
	void assignTexture(smInt p_textureId){
		smTextureAttachment attachment;
		attachment.textureId=p_textureId;
		if(p_textureId>0)
			textureIds.push_back(attachment);
	}

	/// \brief assign the texture
	void assignTexture(smChar *p_referenceName){
		smInt textureId;
		smTextureAttachment attachment;
		if(smTextureManager::findTextureId(p_referenceName,textureId)==SIMMEDTK_TEXTURE_OK){
			attachment.textureId=textureId;
			textureIds.push_back(attachment);
		}
	}

	/// \brief update the original texture vertices with the current
	void updateOriginalVertsWithCurrent();
};

/// \brief: this is a generic Mesh class from which surface and volume meshes are inherited
/// Note: this class cannot exist on its own
class smMesh:public smBaseMesh{

protected:

public:
	smInt  nbrTriangles; ///< number of triangles
	smTriangle *triangles; ///< list of triangles
	smTexCoord *texCoordForTrianglesOBJ; ///< !! tansel for OBJ
	int	nbrTexCoordForTrainglesOBJ; ///< !! tansel for OBJ
	smVec3<smFloat> *triNormals; ///< triangle normals
	smVec3<smFloat> *vertNormals; ///< vertex normals
	smVec3<smFloat> *triTangents; ///< triangle tangents
	smVec3<smFloat> *vertTangents; ///< vertex tangents
	smBool tangentChannel; ///< !!
	vector< vector<smInt> > vertTriNeighbors; ///< list of neighbors for a trinagle
	vector< vector<smInt> > vertVertNeighbors; ///< list of neighbors for a vertex
	vector<smEdge> edges; ///< list of edges


	///AABBB of the mesh.
	///This value is allocated and computed by only collision detection module
	///Therefore it is initally NULL
	smAABB *triAABBs;

	smMeshType meshType; ///< type of mesh (rigid, deformable etc.)
	smMeshFileType meshFileType; ///< type of input mesh

	static QAtomicInt meshIdCounter; ///< !!

public:
	/// \brief constructor
	smMesh();

	/// \brief destructor
	virtual ~smMesh();

	/// \brief compute the neighbors of the vertex
	void getVertexNeighbors();

	/// \brief compute the neighbors of the trinagle
	void getTriangleNeighbors();

	/// \brief initialize vertex arrays
	smBool initVertexArrays(smInt nbr);

	/// \brief initialize triangle arrays
	smBool initTriangleArrays(smInt nbr);

	/// \brief initialize the neighbors of the vertex
	void initVertexNeighbors();

	/// \brief !!
	void allocateAABBTris();

	/// \brief compute the normal of a triangle
	smVec3<smFloat> calculateTriangleNormal(smInt triNbr);

	/// \brief update the normals of triangles after they moved
	void updateTriangleNormals();

	/// \brief update the normals of vertices after they moved
	void updateVertexNormals();

	/// \brief update AABB after the mesh moved
	void upadateAABB();

	/// \brief update AABB of each triangle after mesh moved
	void updateTriangleAABB();

	/// \brief compute triangle tangents
	void calcTriangleTangents();

	/// \brief compute the tangent give the three vertices
	void calculateTangent(smVec3<smFloat>& p1,smVec3<smFloat>& p2,smVec3<smFloat>& p3,smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3<smFloat>& t);

	/// \brief !!
	void calculateTangent_test(smVec3<smFloat>& p1,smVec3<smFloat>& p2,smVec3<smFloat>& p3,smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3<smFloat>& t);

	/// \brief find the neighbors of all vertices of mesh
	void calcNeighborsVertices();

	/// \brief find all the edges of the mesh
	void calcEdges();

	/// \brief translate the mesh
	void translate(smFloat,smFloat,smFloat);

	/// \brief translate the mesh
	void translate(smVec3<smFloat> p_offset);

	/// \brief scale the mesh
	void scale(smVec3<smFloat> p_scaleFactors);

	/// \brief rotate the mesh
	void rotate(smMatrix33<smFloat> p_rot);

	/// \brief check if there is a consistent orientation of triangle vertices
	/// across the entire surface mesh
	void checkCorrectWinding();

	/// \brief get the type fo mesh
	smMeshType getMeshType(){
		return meshType;
	};

	/// \brief load the mesh
	virtual smBool loadMesh(smChar *fileName,smMeshFileType fileType)=0;

	/// \brief render the surface mesh
	virtual void draw(smDrawParam p_params);
};

/// \brief holds the texture co-ordinates
struct smTexCoord{
	smFloat u,v;
};

/// \brief holds the vertex indices of triangle
struct smTriangle{
	smUInt vert[3];
};

/// \brief holds the vertex indices of tetrahedron
struct smTetrahedra{
	smInt vert[4];
};

/// \brief holds the vertex indices of edge
struct smEdge{
	smUInt vert[2];
};

/// \brief !!
class smLineMesh:public smBaseMesh{

public: 
	smAABB *edgeAABBs;///< AABBs for the edges in the mesh
	smEdge *edges;///< edges of the line mesh
	smInt nbrEdges;///< number of edges of the line mesh

	/// \brief destructor
	~smLineMesh(){
		delete[]vertices;
		delete[]origVerts;
		delete[]edgeAABBs;
		delete[]texCoord;
		delete[]edges;
	}

	/// \brief constructor
	smLineMesh(smInt p_nbrVertices):smBaseMesh(){
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

	/// \brief constructor
	smLineMesh(smInt p_nbrVertices, smBool autoEdge):smBaseMesh(){
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

	/// \brief !!
	void createAutoEdges(){
		for(smInt i=0;i<nbrEdges;i++){
			edges[i].vert[0]=i;
			edges[i].vert[1]=i+1;
		}
	}

	/// \brief !!
	virtual void createCustomEdges(){};

	/// \brief updat AABB when the mesh moves
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

		for(smInt i=0;i<nbrEdges;i++){
			///min
			edgeAABBs[i].aabbMin.x = SIMMEDTK_MIN(vertices[edges[i].vert[0]].x,vertices[edges[i].vert[1]].x );
			edgeAABBs[i].aabbMin.y = SIMMEDTK_MIN(vertices[edges[i].vert[0]].y,vertices[edges[i].vert[1]].y );
			edgeAABBs[i].aabbMin.z = SIMMEDTK_MIN(vertices[edges[i].vert[0]].z,vertices[edges[i].vert[1]].z );
			edgeAABBs[i].aabbMin+=minEdgeOffset;
			tempAABB.aabbMin.x=SIMMEDTK_MIN(tempAABB.aabbMin.x,edgeAABBs[i].aabbMin.x);
			tempAABB.aabbMin.y=SIMMEDTK_MIN(tempAABB.aabbMin.y,edgeAABBs[i].aabbMin.y);
			tempAABB.aabbMin.z=SIMMEDTK_MIN(tempAABB.aabbMin.z,edgeAABBs[i].aabbMin.z);

			///max
			edgeAABBs[i].aabbMax.x = SIMMEDTK_MAX(vertices[edges[i].vert[0]].x,vertices[edges[i].vert[1]].x );
			edgeAABBs[i].aabbMax.y = SIMMEDTK_MAX(vertices[edges[i].vert[0]].y,vertices[edges[i].vert[1]].y );
			edgeAABBs[i].aabbMax.z = SIMMEDTK_MAX(vertices[edges[i].vert[0]].z,vertices[edges[i].vert[1]].z );
			edgeAABBs[i].aabbMax+=maxEdgeOffset;
			tempAABB.aabbMax.x=SIMMEDTK_MAX(tempAABB.aabbMax.x,edgeAABBs[i].aabbMax.x);
			tempAABB.aabbMax.y=SIMMEDTK_MAX(tempAABB.aabbMax.y,edgeAABBs[i].aabbMax.y);
			tempAABB.aabbMax.z=SIMMEDTK_MAX(tempAABB.aabbMax.z,edgeAABBs[i].aabbMax.z);
		}

		tempAABB.aabbMin+=minOffset;
		tempAABB.aabbMax+=maxOffset;
		aabb=tempAABB;
	}

	/// \brief translate the vertices of mesh
	void translate(smFloat p_offsetX,smFloat p_offsetY,smFloat p_offsetZ){

		for(smInt i=0; i<nbrVertices; i++){
			vertices[i].x=vertices[i].x+p_offsetX;
			vertices[i].y=vertices[i].y+p_offsetY;
			vertices[i].z=vertices[i].z+p_offsetZ;
		}
		updateAABB();
	}

	/// \brief translate the vertices of mesh
	void translate(smVec3<smFloat> p_offset){

		for(smInt i=0; i<nbrVertices; i++){
			vertices[i]=vertices[i]+p_offset;
			origVerts[i]=origVerts[i]+p_offset;
		}
		updateAABB();
	}

	/// \brief scale the mesh
	void scale(smVec3<smFloat> p_scaleFactors){

		for(smInt i=0; i<nbrVertices; i++){
			vertices[i].x=vertices[i].x*p_scaleFactors.x;
			vertices[i].y=vertices[i].y*p_scaleFactors.y;
			vertices[i].z=vertices[i].z*p_scaleFactors.z;

			origVerts[i].x=origVerts[i].x*p_scaleFactors.x;
			origVerts[i].y=origVerts[i].y*p_scaleFactors.y;
			origVerts[i].z=origVerts[i].z*p_scaleFactors.z;
		}
		updateAABB();
	}

	/// \brief rotate the mesh
	void rotate(smMatrix33<smFloat> p_rot){

		for(smInt i=0; i<nbrVertices; i++){
			vertices[i]=p_rot*vertices[i];
			origVerts[i]=p_rot*origVerts[i];
		}
		updateAABB();
	}

	/// \brief query if the mesh is textured
	inline smBool isMeshTextured(){
		return isTextureCoordAvailable;
	}

	/// \brief draw the mesh
	void draw(smDrawParam p_params);
};

#endif
