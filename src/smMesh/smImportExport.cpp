#include "smMesh/smImportExport.h"
#include "smMesh/smMesh.h"
#include "smCore/smSDK.h"

/// \brief
smBool smImportExport::convertToJSON(smMesh *p_mesh, smChar *p_outputFileName,
                                     smExportOptions p_export){

	FILE *file;
	file=fopen(p_outputFileName,"w");
	fprintf(file,"{ \n");

	if(file==NULL)
		return false;

	fprintf(file,"\t\"vertexPositions\" : \n[");
	for(int i=0;i<p_mesh->nbrVertices-1;i++){
		fprintf(file,"%f,%f,%f,",p_mesh->vertices[i].x,p_mesh->vertices[i].y,p_mesh->vertices[i].z);
	}
	fprintf(file,"%f,%f,%f",p_mesh->vertices[p_mesh->nbrVertices-1].x,p_mesh->vertices[p_mesh->nbrVertices-1].y,p_mesh->vertices[p_mesh->nbrVertices-1].z);
	fprintf(file,"],\n");

	fprintf(file,"\t\"vertexNormals\" : \n[");
		for(int i=0;i<p_mesh->nbrVertices-1;i++){
			fprintf(file,"%f,%f,%f,",p_mesh->vertNormals[i].x,p_mesh->vertNormals[i].y,p_mesh->vertNormals[i].z);
		}
	fprintf(file,"%f,%f,%f",p_mesh->vertNormals[p_mesh->nbrVertices-1].x,p_mesh->vertNormals[p_mesh->nbrVertices-1].y,p_mesh->vertNormals[p_mesh->nbrVertices-1].z);
	fprintf(file,"],\n");

	fprintf(file,"\t\"vertexTextureCoords\" : [");
	for(int i=0;i<p_mesh->nbrVertices-1;i++){
		fprintf(file,"%f,%f,",p_mesh->texCoord[i].u,p_mesh->texCoord[i].v);
	}
		fprintf(file,"%f,%f",p_mesh->texCoord[p_mesh->nbrVertices-1].u,p_mesh->texCoord[p_mesh->nbrVertices-1].v);
	fprintf(file,"],\n");

	fprintf(file,"\t\"tangents\" : \n[");
	for(int i=0;i<p_mesh->nbrVertices-1;i++){
		fprintf(file,"%f,%f,%f,",p_mesh->vertTangents[i].x,p_mesh->vertTangents[i].y,p_mesh->vertTangents[i].z);
	}
	fprintf(file,"%f,%f,%f",p_mesh->vertTangents[p_mesh->nbrVertices-1].x,p_mesh->vertTangents[p_mesh->nbrVertices-1].y,p_mesh->vertTangents[p_mesh->nbrVertices-1].z);
	fprintf(file,"],\n");

	fprintf(file,"\t\"indices\" : \n[");
	for(int i=0;i<p_mesh->nbrTriangles;i++){
		fprintf(file,"%d,%d,%d,",p_mesh->triangles[i].vert[0],p_mesh->triangles[i].vert[1],p_mesh->triangles[i].vert[2]);
	}
	fprintf(file,"%d,%d,%d",p_mesh->triangles[p_mesh->nbrTriangles-1].vert[0],p_mesh->triangles[p_mesh->nbrTriangles-1].vert[1],p_mesh->triangles[p_mesh->nbrTriangles-1].vert[2]);
	fprintf(file,"]\n");

	fprintf(file,"}");

	fclose(file);
	return true;
}
