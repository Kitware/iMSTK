#include "smMesh/smVolumeMesh.h"


///brief: constructor
smVolumeMesh::smVolumeMesh(smMeshType p_meshtype,smErrorLog *log=NULL)
{
	this->log_VM=log;
	meshType = p_meshtype;
	
}

///brief: loads the specified volume mesh
smBool smVolumeMesh::loadMesh(smChar *fileName,smMeshFileType fileType=SM_FILETYPE_VOLUME)
{
	smBool ret;
	if(fileType==SM_FILETYPE_VOLUME)
	{
		ret = LoadTetra(fileName);
		if(ret==0)
			if(log_VM!=NULL)
				log_VM->addError(this,"Error: Mesh file NOT FOUND");
	}
	else
	{
		if(log_VM!=NULL)
			log_VM->addError(this,"Error: Mesh file type unidentified");
		ret = 0;
	}

	return ret;

}

void smVolumeMesh::translateVolumeMesh(smVec3<smFloat> p_offset){
	this->translate(p_offset);
	for(smInt i=0;i<this->nbrNodes;i++){
		nodes[i]+=p_offset;
		
	
	}




}
void smVolumeMesh::scaleVolumeMesh(smVec3<smFloat> p_offset){
	scale(p_offset);
	for(smInt i=0;i<this->nbrNodes;i++){
		nodes[i].x=nodes[i].x*p_offset.x;
		nodes[i].y=nodes[i].y*p_offset.y;
		nodes[i].z=nodes[i].z*p_offset.z;
		
	
	}




}


void smVolumeMesh::rotVolumeMesh(smMatrix33<smFloat> p_rot){
	rotate(p_rot);

	for(smInt i=0; i<nbrNodes; i++)
	{
		nodes[i]=p_rot*nodes[i];
	}

	

}

///brief: loads the tetra mesh from abacus
///Extensions to support other formats will come soon...
smBool smVolumeMesh::LoadTetra(const smChar *fileName){

	smFloat number;
	smFloat numnodes;
	smChar comma;
	smChar stri[19];
	smInt i;

	FILE *fp = fopen (fileName,"rb");
	if(!fp)
		return false;


	fscanf(fp,"%f%c\n",&numnodes,&comma);
	nbrNodes = numnodes;
	nodes = new smVec3<smFloat>[nbrNodes];
	fixed = new smBool[nbrNodes];

	for(i=0; i<nbrNodes; i++){
		fixed[i]=false;
	}

	smFloat *nodeNumber = new smFloat[nbrNodes];
	for(i=0; i<nbrNodes; i++){
		fscanf(fp,"%f",&number);
		nodeNumber[i]=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		nodes[i].x=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		nodes[i].y=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		nodes[i].z=number;
		fscanf(fp,"\n");
	}


	fscanf(fp,"%s\n",stri);
	nbrTetra=563;//2500
	//nbrTetra=3500;//2500
	tetra = new smTetrahedra[nbrTetra];

	for(i=0; i<nbrTetra; i++){
		fscanf(fp,"%f",&number);
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		tetra[i].vert[0]=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		tetra[i].vert[1]=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		tetra[i].vert[2]=number;
		fscanf(fp,"%c\n",&comma);
		fscanf(fp,"%f",&number);
		tetra[i].vert[3]=number;
		fscanf(fp,"\n");
	}
	fclose(fp);

}

///brief: loads the surface vertices and triangles
smBool smVolumeMesh::getSurface(const smChar *fileName){

	smFloat number;
	smInt j,i;
	smChar comma;
	smInt count;
	nbrTriangles = 777-564+1;//790....read here automatically
	//nbrTriangles = 790;//790....read here automatically
	smTriangle *surfaceRelation = new smTriangle[nbrTriangles];

	initTriangleArrays(nbrTriangles);
	
	FILE *fp = fopen (fileName,"rb");
	if(!fp)
		return false;


	smChar stri[19];
	fscanf(fp,"%s\n",stri);
	for(i=0; i<nbrTriangles; i++){
		fscanf(fp,"%f",&number);
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		surfaceRelation[i].vert[0]=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		surfaceRelation[i].vert[1]=number;
		fscanf(fp,"%c",&comma);
		fscanf(fp,"%f",&number);
		surfaceRelation[i].vert[2]=number;
		fscanf(fp,"\n");
	}
	fclose(fp);

	smBool *onSurface = new smBool[nbrNodes];
	for(j=0; j<nbrNodes; j++)
		onSurface[j] = false;

	for(j=0; j<nbrTriangles; j++){
		onSurface[surfaceRelation[j].vert[0]-1]=onSurface[surfaceRelation[j].vert[1]-1]
												=onSurface[surfaceRelation[j].vert[2]-1]=true;
	}

	count = 0;
	smInt *temp = new smInt[nbrNodes];
	for(i=0; i<nbrNodes; i++){
		if(onSurface[i]){			  
			temp[i]=count;  
			count++;			  
		}
		else
			temp[i]=-1;
	}

	initVertexArrays(count);
	
	for(i=0; i<nbrTriangles; i++){

		triangles[i].vert[0] = temp[surfaceRelation[i].vert[0]-1];

		triangles[i].vert[1] = temp[surfaceRelation[i].vert[1]-1];

		triangles[i].vert[2] = temp[surfaceRelation[i].vert[2]-1];

	}

	surfaceNodeIndex = new smInt[nbrVertices];

	count = 0;
	for(i=0; i<nbrNodes; i++){
		if(onSurface[i]){
			surfaceNodeIndex[count] = i;
			count++;
		}		
	}



	//copy the co-ordinates of the surface vertices	
	initSurface();	

	delete []temp;
	delete []onSurface;
	delete []surfaceRelation;

}

///brief: loads the tetra mesh from abacus
///Extensions to support other formats will come soon...
smBool smVolumeMesh::readBC(const smChar *fileName){

	smInt node;
	smInt number;
	smChar comma;
	smInt i;

	FILE *fp = fopen (fileName,"rb");
	if(!fp)
		return false;
	
	//smInt numNodes=16;//311
	smInt numNodes=311;//311
	
	for(i=0; i<numNodes; i++){
		fscanf(fp,"%d",&number);
		fixed[number-1] = true;
		fscanf(fp,"%c",&comma);
	}

	fclose(fp);
}

//brief: copies the updated co-ordinates of the surface vertices only
void smVolumeMesh::copySurface(){

	smInt i;

	for(i=0; i<nbrVertices; i++){ 		
		vertices[i].x = nodes[surfaceNodeIndex[i]].x;
		vertices[i].y = nodes[surfaceNodeIndex[i]].y;
		vertices[i].z = nodes[surfaceNodeIndex[i]].z;
	}

	updateTriangleNormals();
	updateVertexNormals();

}

//brief: copies the updated co-ordinates of the surface vertices only
void smVolumeMesh::initSurface(){

	smInt i;

	for(i=0; i<nbrVertices; i++){ 		
		vertices[i].x = nodes[surfaceNodeIndex[i]].x;
		vertices[i].y = nodes[surfaceNodeIndex[i]].y;
		vertices[i].z = nodes[surfaceNodeIndex[i]].z;

		origVerts[i].x = nodes[surfaceNodeIndex[i]].x;
		origVerts[i].y = nodes[surfaceNodeIndex[i]].y;
		origVerts[i].z = nodes[surfaceNodeIndex[i]].z;
	}
	initVertexNeighbors();
	updateTriangleNormals();
	updateVertexNormals();

}

///brief: destructor
smVolumeMesh::~smVolumeMesh()
{
	delete [] tetra;
}
