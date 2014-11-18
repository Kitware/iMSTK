#include <assert.h>
#include <map>
using std::map;
//assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "smMesh/smSurfaceMesh.h"

///brief: constructor
smSurfaceMesh::smSurfaceMesh(smMeshType p_meshtype,smErrorLog *log=NULL)
{
	this->log_SF=log;
	meshType=p_meshtype;
	meshFileType = SM_FILETYPE_NONE;
}


///brief: destructor
smSurfaceMesh::~smSurfaceMesh()
{

}
///brief: loads the mesh based on the file type and initializes the normals
smBool smSurfaceMesh::loadMesh(smChar *fileName,smMeshFileType fileType)
{
	smBool ret = true;
	switch (fileType) {
	case SM_FILETYPE_3DS:
	case SM_FILETYPE_OBJ:
		meshFileType = fileType;
		ret = LoadMeshAssimp(fileName);
		break;
	default:
		if(log_SF!=NULL)
			log_SF->addError(this,"Error: Mesh file TYPE UNIDENTIFIED");
		ret = false;
	}
	assert(ret);
	
	if (ret == false) {
		if (log_SF != NULL)
			log_SF->addError(this, "Error: Mesh file NOT FOUND");
	}
	if(ret){
		initVertexNeighbors();  
		this->updateTriangleNormals();
		this->updateVertexNormals();

		//edge information
		this->calcNeighborsVertices();
		this->calcEdges();
		this->upadateAABB();
	}
	return ret;
}

smBool smSurfaceMesh::LoadMeshAssimp(const smChar *fileName)
{
	//Tell Assimp to not import any of the following from the mesh it loads
	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
		aiComponent_CAMERAS | aiComponent_LIGHTS | 
		aiComponent_MATERIALS | aiComponent_TEXTURES | 
		aiComponent_BONEWEIGHTS | aiComponent_COLORS |
		aiComponent_TANGENTS_AND_BITANGENTS |
		aiComponent_NORMALS | aiComponent_ANIMATIONS);

	//Import the file, and do some post-processing
	const aiScene* scene = importer.ReadFile(fileName,
		//aiProcess_CalcTangentSpace | //Calculates the tangents and bitangents
		aiProcess_Triangulate | //Triangulate any polygons that are not triangular
		aiProcess_JoinIdenticalVertices | //Ensures indexed vertices from faces
		aiProcess_RemoveComponent | //Removes the components in AI_CONFIG_PP_RVC_FLAGS
		 //aiProcess_GenNormals | //Generate normals for all the faces of the mesh
		aiProcess_ImproveCacheLocality); //Reorders triangles for better vertex cache locality
	
	if (scene == NULL) {
		if (log_SF != NULL)
			log_SF->addError(this, string("Error: Error loading mesh: ") + string(fileName));
		return false;
	}

	//extract the information from the aiScene's mesh objects
	aiMesh *mesh = scene->mMeshes[0]; //Guarenteed to have atleast one mesh

	if (mesh->HasTextureCoords(0)) {
		this->isTextureCoordAvailable = 1;
	}
	else {
		this->isTextureCoordAvailable = 0;
	}

	initVertexArrays(mesh->mNumVertices);
	initTriangleArrays(mesh->mNumFaces);

	//Get indexed vertex data
	for (int i = 0; i < mesh->mNumVertices; i++) {
		this->vertices[i].x = mesh->mVertices[i].x;
		this->vertices[i].y = mesh->mVertices[i].y;
		this->vertices[i].z = mesh->mVertices[i].z;
	}

	//Get indexed texture coordinate data
	if (isTextureCoordAvailable) {
		//Assimp supports 3D texture coords, but we only support 2D
		if (mesh->mNumUVComponents[0] != 2) {
			if (log_SF != NULL)
				log_SF->addError(this, "Error: Error loading mesh, non-two dimensional texture coordinate found.");
			this->isTextureCoordAvailable = 0;
			return false;
		}
		//Extract the texture data
		for (smUInt i = 0; i < mesh->mNumVertices; i++) {
			this->texCoord[i].u = mesh->mTextureCoords[0][i].x;
			this->texCoord[i].v = mesh->mTextureCoords[0][i].y;
		}
	}

	//Setup triangle/face data
	for (int i = 0; i < mesh->mNumFaces; i++) {
		if (mesh->mFaces[i].mNumIndices != 3) { //Make sure that the face is triangular
			if (log_SF != NULL)
				log_SF->addError(this, "Error: Error loading mesh, non-triangular face found.");
			//might want to consider an assert here also
			return false;
		}
		this->triangles[i].vert[0] = mesh->mFaces[i].mIndices[0];
		this->triangles[i].vert[1] = mesh->mFaces[i].mIndices[1];
		this->triangles[i].vert[2] = mesh->mFaces[i].mIndices[2];
	}

	return true;
}
