#ifndef SMVAO_H
#define SMVAO_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smConfigRendering.h"
#include "smUtilities/smVec3.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smUtils.h"
#include <GL/glut.h>
#include <GL/glew.h>

#include "assert.h"
#include <QHash>
#include <QMultiHash>
#include "smRendering/smVBO.h"
#include "smRendering/smVAO.h"
#include "smShader/smShader.h"

enum smVBOBufferType{
	SMVBO_POS,
	SMVBO_NORMALS,
	SMVBO_TEXTURECOORDS,
	SMVBO_TANGENTS,
	SMVBO_INDEX,
	SMVBO_VEC4F,
	SMVBO_VEC3F,
	SMVBO_VEC2F
};

struct smVBOBufferEntryInfo{
	///attrib index; 0, 1, 2. It starts from 0
	smInt attributeIndex;
	///based on type the data buffer  will change. It may be Position, normals, texture coords, tangents
	smVBOBufferType arrayBufferType;
	///pointer to the actual that. It is mesh data.
	void *attribPointer;
	///total number of elements
	smInt nbrElements;
	///total size of elements in bytes.
	smInt size;
	///attribName in the shader
	string   shaderAttribName;
	GLint    shaderAttribLocation;
public:
	smVBOBufferEntryInfo(){
		shaderAttribLocation=-1;
		attributeIndex-1;
		attribPointer=NULL;
		nbrElements=0;
		arrayBufferType=SMVBO_POS;
	}
};

class smVAO:public smCoreClass{
private:
	GLuint vboDataId;
	GLuint vboIndexId;
	QHash<smInt,smInt> dataOffsetMap;
	QHash<smInt,smInt> indexOffsetMap;
	QHash<smInt,smInt> numberofVertices;
	QHash<smInt,smInt> numberofTriangles;
	smErrorLog *log;
	smBool renderingError;
	smShader *shader;
	///Used for attaching attribs to the vertex objects
	smBool bindShaderObjects;
public:
	GLuint VAO;
	smInt totalNbrBuffers;
	GLuint bufferIndices[SOFMIS_MAX_VBOBUFFERS];
	smInt indexBufferLocation;///stores the index buffer location in the bufferIndices array to easy access
	smVBOBufferEntryInfo bufferInfo[SOFMIS_MAX_VBOBUFFERS];
	smVBOType vboType;
	smChar error[500];
	///All VBOs are stored here
	static QHash<smInt,smVAO *>VAOs;
	smMesh *mesh;

	///need error log and totalBuffer Size
	smVAO(smErrorLog *p_log, smVBOType p_vboType=SOFMIS_VBO_DYNAMIC, smBool p_bindShaderObjects=true){
		this->log=p_log;
		renderingError=false;
		totalNbrBuffers=0;
		vboType=p_vboType;
		VAOs.insert(this->uniqueId.ID,this);
		indexBufferLocation=-1;
		bindShaderObjects=p_bindShaderObjects;
	}

	void setBufferData(smVBOBufferType p_type,string p_ShaderAttribName, smInt p_nbrElements, void *p_ptr){
		bufferInfo[totalNbrBuffers].arrayBufferType=p_type;
		if(p_type==SMVBO_POS ||
		   p_type==SMVBO_NORMALS ||
		   p_type==SMVBO_TANGENTS ||
		   p_type==SMVBO_VEC3F) {
			bufferInfo[totalNbrBuffers].size=sizeof(smVec3f)*p_nbrElements;
		}
		else if(p_type==SMVBO_TEXTURECOORDS||
		        p_type==SMVBO_VEC2F){
			bufferInfo[totalNbrBuffers].size=sizeof(smTexCoord)*p_nbrElements;
		}
		else if(p_type==SMVBO_VEC4F){
			bufferInfo[totalNbrBuffers].size=sizeof(smFloat)*4*p_nbrElements;
		}

		bufferInfo[totalNbrBuffers].attribPointer=p_ptr;
		bufferInfo[totalNbrBuffers].nbrElements=p_nbrElements;
		bufferInfo[totalNbrBuffers].attributeIndex=totalNbrBuffers;
		bufferInfo[totalNbrBuffers].shaderAttribName=p_ShaderAttribName;
		totalNbrBuffers++;
	}

	void setTriangleInfo(string p_ShaderAttribName, smInt p_nbrTriangles, void *p_ptr){
		bufferInfo[totalNbrBuffers].arrayBufferType=SMVBO_INDEX;
		bufferInfo[totalNbrBuffers].nbrElements=p_nbrTriangles*3;
		bufferInfo[totalNbrBuffers].attribPointer=p_ptr;
		bufferInfo[totalNbrBuffers].size=sizeof(smInt)*p_nbrTriangles*3;
		bufferInfo[totalNbrBuffers].shaderAttribName=p_ShaderAttribName;
		totalNbrBuffers++;
	}

	///uses default attrib location
	smBool setBufferDataFromMesh(smMesh *p_mesh,
	                             smShader *p_shader,
	                             string p_POSITIONShaderName="Position",
	                             string p_NORMALShaderName="Normal",
	                             string p_TEXTURECOORDShaderName="texCoords",
	                             string p_TANGENTSName="Tangents"){

		if(p_shader==NULL)
			shader=smShader::getShader(p_mesh->renderDetail.shaders[0]);
		else
			shader=p_shader;

		bufferInfo[totalNbrBuffers].arrayBufferType=SMVBO_POS;
		bufferInfo[totalNbrBuffers].size=sizeof(smVec3f)*p_mesh->nbrVertices;
		bufferInfo[totalNbrBuffers].attribPointer=p_mesh->vertices;
		bufferInfo[totalNbrBuffers].nbrElements=p_mesh->nbrVertices;
		bufferInfo[totalNbrBuffers].attributeIndex=totalNbrBuffers;
		bufferInfo[totalNbrBuffers].shaderAttribName=p_POSITIONShaderName;
		totalNbrBuffers++;

		bufferInfo[totalNbrBuffers].arrayBufferType=SMVBO_NORMALS;
		bufferInfo[totalNbrBuffers].size=sizeof(smVec3f)*p_mesh->nbrVertices;
		bufferInfo[totalNbrBuffers].attribPointer=p_mesh->vertNormals;
		bufferInfo[totalNbrBuffers].nbrElements=p_mesh->nbrVertices;
		bufferInfo[totalNbrBuffers].attributeIndex=totalNbrBuffers;
		bufferInfo[totalNbrBuffers].shaderAttribName=p_NORMALShaderName;
		totalNbrBuffers++;

		///texture coord is for each vertex 
		bufferInfo[totalNbrBuffers].arrayBufferType=SMVBO_TEXTURECOORDS;
		bufferInfo[totalNbrBuffers].size=sizeof(smTexCoord)*p_mesh->nbrVertices;
		bufferInfo[totalNbrBuffers].attribPointer=p_mesh->texCoord;
		bufferInfo[totalNbrBuffers].nbrElements=p_mesh->nbrVertices;
		bufferInfo[totalNbrBuffers].attributeIndex=totalNbrBuffers;
		bufferInfo[totalNbrBuffers].shaderAttribName=p_TEXTURECOORDShaderName;
		totalNbrBuffers++;

		if(p_mesh->tangentChannel){
			bufferInfo[totalNbrBuffers].arrayBufferType=SMVBO_TANGENTS;
			bufferInfo[totalNbrBuffers].size=sizeof(smVec3f)*p_mesh->nbrVertices;
			bufferInfo[totalNbrBuffers].attribPointer=p_mesh->vertTangents;
			bufferInfo[totalNbrBuffers].nbrElements=p_mesh->nbrVertices;
			bufferInfo[totalNbrBuffers].attributeIndex=totalNbrBuffers;
			bufferInfo[totalNbrBuffers].shaderAttribName=p_TANGENTSName;
			totalNbrBuffers++;
		}

		bufferInfo[totalNbrBuffers].arrayBufferType=SMVBO_INDEX;
		bufferInfo[totalNbrBuffers].nbrElements=p_mesh->nbrTriangles*3;
		bufferInfo[totalNbrBuffers].attribPointer=p_mesh->triangles;
		bufferInfo[totalNbrBuffers].size=sizeof(smInt)*p_mesh->nbrTriangles*3;
		bufferInfo[totalNbrBuffers].attributeIndex=totalNbrBuffers;
		totalNbrBuffers++;

		mesh=p_mesh;
		return true;
	}

	smBool updateStreamData();
	static void initVAOs(smDrawParam p_param){
		foreach (smVAO *vao, VAOs)
			vao->initBuffers(p_param);
	}

	void initBuffers(smDrawParam p_param);
	static inline smVAO * getVAO(smUnifiedID p_shaderID){
		return VAOs[p_shaderID.ID];
	}

	inline void enable(){
		glBindVertexArray(VAO);
	}

	inline void disable(){
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}

	void draw(smDrawParam p_params);

	~smVAO(){
		glDeleteBuffers(totalNbrBuffers, bufferIndices);
		glDeleteVertexArrays(1, &VAO);
	}
};

#endif
