#ifndef SMVBO_H 
#define SMVBO_H 

#include <QHash>
#include <GL/glew.h>
#include <GL/glut.h>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smConfigRendering.h"
#include "smUtilities/smVec3.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smUtils.h"
#include "assert.h"

/////The configuration for VBO
//enum smVBOType{
//	SOFMIS_VBO_STATIC,
//	SOFMIS_VBO_NOINDICESCHANGE,
//	SOFMIS_VBO_DYNAMIC,
//};
//
/////Vertex Buffer Objects Return Types
//enum smVBOResult{
//	SOFMIS_VBO_OK,
//	SOFMIS_VBO_NODATAMEMORY,
//	SOFMIS_VBO_NOINDEXMEMORY,
//	SOFMIS_VBO_INVALIDOPERATION,
//	SOFMIS_VBO_BUFFERPOINTERERROR
//};



class smVBO:public smCoreClass{
private:
	smInt currentDataOffset;
	smInt currentIndexOffset;
	smInt sizeOfDataBuffer;
	smInt sizeOfIndexBuffer;
	smVBOType vboType;
	GLuint vboDataId;
	GLuint vboIndexId;
	QHash<smInt,smInt> dataOffsetMap;
	QHash<smInt,smInt> indexOffsetMap;
	QHash<smInt,smInt> numberofVertices;
	QHash<smInt,smInt> numberofTriangles;

	smErrorLog *log;
	smBool renderingError;


public:
	smVBO(smErrorLog *p_log){
		this->log=p_log;
		renderingError=false;

	}

	void init(smVBOType p_vboType){
		smChar error[200];
		glGenBuffersARB(1,&vboDataId);
		glGenBuffersARB(1,&vboIndexId);
		assert(vboDataId>0);
		assert(vboIndexId>0);


		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboDataId);
		if(p_vboType==SOFMIS_VBO_STATIC)
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, SOFMIS_VBOBUFFER_DATASIZE, 0, GL_STATIC_DRAW);
		else if(p_vboType==SOFMIS_VBO_DYNAMIC||p_vboType==SOFMIS_VBO_NOINDICESCHANGE)
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, SOFMIS_VBOBUFFER_DATASIZE, 0, GL_STREAM_DRAW);

		SM_CHECKERROR(log,error)

		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndexId);
		if(p_vboType==SOFMIS_VBO_STATIC||p_vboType==SOFMIS_VBO_NOINDICESCHANGE)
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, SOFMIS_VBOBUFFER_INDEXSIZE, 0, GL_STATIC_DRAW);
		else if(p_vboType==SOFMIS_VBO_DYNAMIC)
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, SOFMIS_VBOBUFFER_INDEXSIZE, 0, GL_STREAM_DRAW);

		SM_CHECKERROR(log,error)
		vboType=p_vboType;
		sizeOfDataBuffer=SOFMIS_VBOBUFFER_DATASIZE;
		sizeOfIndexBuffer=SOFMIS_VBOBUFFER_INDEXSIZE;
		currentDataOffset=0;
		currentIndexOffset=0;
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	}



	smVBOResult addVerticestoBuffer(smInt p_nbrVertices,smInt p_nbrTriangles,smInt p_objectId){

		if(sizeof(smVec3<smFloat>)*p_nbrVertices+sizeof(smVec3<smFloat>)*p_nbrVertices+sizeof(smTexCoord)*p_nbrVertices>sizeOfDataBuffer-currentDataOffset)
			return SOFMIS_VBO_NODATAMEMORY;
		if(sizeof(smInt)*p_nbrTriangles*3>sizeOfIndexBuffer-currentIndexOffset)
			return SOFMIS_VBO_NODATAMEMORY;

		dataOffsetMap[p_objectId]=currentDataOffset;
		indexOffsetMap[p_objectId]=currentIndexOffset;
		numberofVertices[p_objectId]=p_nbrVertices;
		numberofTriangles[p_objectId]=p_nbrTriangles;
		///add the vertices and normals and the texture coordinates
		currentDataOffset+=sizeof(smVec3<smFloat>)*p_nbrVertices+sizeof(smVec3<smFloat>)*p_nbrVertices+sizeof(smTexCoord)*p_nbrVertices;
		currentIndexOffset+=p_nbrTriangles*sizeof(smTriangle);
		
		
		return SOFMIS_VBO_OK;
	}
	

	smVBOResult updateVertices(smVec3<smFloat> *p_vectors,smVec3<smFloat> *p_normals,smTexCoord *p_textureCoords, smInt p_objectId);
	smVBOResult updateTriangleIndices(smInt *p_indices,smInt p_objectId);
	smVBOResult drawElements(smInt p_objectId);
	
	///update the static vertices initially
	smVBOResult initStaticVertices(smVec3<smFloat> *p_vectors,smVec3<smFloat> *p_normals,smTexCoord *p_textureCoords, smInt p_objectId);
	
	///update the static triangle indices initially
	smVBOResult initTriangleIndices(smInt *p_indices,smInt p_objectId);




	///deletion of the VBO buffers
	~smVBO(){
		glDeleteBuffersARB(1, &vboDataId);
		glDeleteBuffersARB(1, &vboIndexId);
	}



};



#endif