#include "smMesh/smMesh.h"
#include "smCore/smSDK.h"
#include "smRendering/smGLRenderer.h"

QAtomicInt smMesh::meshIdCounter(1);

smBaseMesh::smBaseMesh(){
	smSDK::registerMesh(this);
}

void smBaseMesh::updateOriginalVertsWithCurrent(){

	for(smInt i=0;i<nbrVertices;i++){
		origVerts[i]=vertices[i];
	}
}

/// \brief constructor
smMesh::smMesh(){

	vertices=0;
	triangles=0;
	texCoord=0;
	triNormals=0;
	vertNormals=0;
	triTangents=0;
	vertTangents=0;
	type=SOFMIS_SMMESH;
	isTextureCoordAvailable=false;
	tangentChannel=false;
	triAABBs=NULL;
}

/// \brief destructor
smMesh::~smMesh(){

	delete [] vertices;
	delete [] triangles;
	delete [] texCoord;
	delete [] triNormals;
	delete [] vertNormals;
	delete [] triTangents;
	delete [] vertTangents;
	if(triAABBs!=NULL)
		delete[] triAABBs;
}

/// \brief
void smMesh::allocateAABBTris(){

	if(triAABBs==NULL)
		triAABBs=new smAABB[nbrTriangles];
}

/// \brief
void CalculateTangentArray(smInt vertexCount, const smVec3f *vertex,
                           const smVec3f *normal, const smTexCoord *texcoord,
                           long triangleCount, const smTriangle *triangle,
                           smVec3f *tangent){

	smVec3f *tan1 = new smVec3f[vertexCount * 2];
	smVec3f *tan2 = tan1 + vertexCount;
	memset(tan1, 0, vertexCount * sizeof(smVec3f) * 2);

	for (long a = 0; a < triangleCount; a++){
		long i1 = triangle->vert[0];
		long i2 = triangle->vert[1];
		long i3 = triangle->vert[2];
		
		const smVec3f& v1 = vertex[i1];
		const smVec3f& v2 = vertex[i2];
		const smVec3f& v3 = vertex[i3];
		
		const smTexCoord& w1 = texcoord[i1];
		const smTexCoord& w2 = texcoord[i2];
		const smTexCoord& w3 = texcoord[i3];
		
		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;
		
		float s1 = w2.u- w1.u;
		float s2 = w3.u - w1.u;
		float t1 = w2.v - w1.v;
		float t2 = w3.v - w1.v;
		
		float r = 1.0F / (s1 * t2 - s2 * t1);
		smVec3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                                              (t2 * z1 - t1 * z2) * r);
		smVec3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                                              (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;

		triangle++;
	}

	for (long a = 0; a < vertexCount; a++){
		smVec3f n = normal[a];
		smVec3f t = tan1[a];
		tangent[a] = (t - n * n.dot(t));
		tangent[a].normalize();
	}
	delete[] tan1;
}

/// \brief calucate the triangle tangents
void smMesh::calcTriangleTangents(){

	smInt t;
	// First calculate the triangle tangents
	for (t = 0; t < nbrTriangles; t++){
		smTriangle *tmpTri = &triangles[t];
		smVec3f *v0 = &vertices[tmpTri->vert[0]];
		smVec3f *v1 = &vertices[tmpTri->vert[1]];
		smVec3f *v2 = &vertices[tmpTri->vert[2]];
		smTexCoord *t0 = &texCoord[tmpTri->vert[0]];
		smTexCoord *t1 = &texCoord[tmpTri->vert[1]];
		smTexCoord *t2 = &texCoord[tmpTri->vert[2]];
		if (this->meshFileType == SM_FILETYPE_3DS){
			calculateTangent(*v2, *v1, *v0, *t2, *t1, *t0, triTangents[t]);
		}
		else if (this->meshFileType == SM_FILETYPE_OBJ){
			calculateTangent_test(*v0, *v1, *v2, *t0, *t1, *t2, triTangents[t]);
		}
	}

	//calculate the vertex normals
	if (this->meshFileType == SM_FILETYPE_3DS || this->meshFileType == SM_FILETYPE_OBJ){
		for (smInt v = 0; v<nbrVertices; v++)
		{
			vertTangents[v].x = vertTangents[v].y = vertTangents[v].z = 0;
			for (t = 0; t<vertTriNeighbors[v].size(); t++)
			{
				vertTangents[v].x += triTangents[(vertTriNeighbors[v])[t]].x;
				vertTangents[v].y += triTangents[(vertTriNeighbors[v])[t]].y;
				vertTangents[v].z += triTangents[(vertTriNeighbors[v])[t]].z;
			}
			vertTangents[v].normalize();
			vertTangents[v] = (vertTangents[v] - vertNormals[v] * vertNormals[v].dot(vertTangents[v]));
			vertTangents[v].normalize();
		}
	}
}

/// \brief calucate the triangle tangent for rendering purposes
void smMesh::calculateTangent(smVec3<smFloat>& p1,smVec3<smFloat>& p2,smVec3<smFloat>& p3,smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3<smFloat>& t){

	smVec3<smFloat> v1;
	smVec3<smFloat> v2;

	v1.x = p2.x-p1.x;
	v1.y = p2.y-p1.y;
	v1.z = p2.z-p1.z;

	v2.x = p3.x-p1.x;
	v2.y = p3.y-p1.y;
	v2.z = p3.z-p1.z;

	smFloat tt1 = t2.u - t1.u;
	smFloat tt2 = t3.u - t1.u;

	smFloat bb1 = t2.v - t1.v;
	smFloat bb2 = t3.v - t1.v;

	t.x = bb2*v1.x - bb1*v2.x;
	t.y = bb2*v1.y - bb1*v2.y;
	t.z = bb2*v1.z - bb1*v2.z;

	t.normalize();
}

/// \brief
void smMesh::calculateTangent_test(smVec3<smFloat>& p1,smVec3<smFloat>& p2,smVec3<smFloat>& p3,smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3<smFloat>& t){

	smVec3<smFloat> v1;
	smVec3<smFloat> v2;

	v1.x = p2.x-p1.x;
	v1.y = p2.y-p1.y;
	v1.z = p2.z-p1.z;

	v2.x = p3.x-p1.x;
	v2.y = p3.y-p1.y;
	v2.z = p3.z-p1.z;

	smFloat tt1 = t2.u - t1.u;
	smFloat tt2 = t3.u - t1.u;

	smFloat bb1 = t2.v - t1.v;
	smFloat bb2 = t3.v - t1.v;
	float r = 1.0F / (tt1 * bb2 - tt2 * bb1);
	t.x = (bb2*v1.x - bb1*v2.x)*r;
	t.y = (bb2*v1.y - bb1*v2.y)*r;
	t.z = (bb2*v1.z - bb1*v2.z)*r;
}

/// \brief calculates the normal of the vertex
void smMesh::updateVertexNormals(){

	smInt j;
	smVec3<smFloat> temp(0.0,0.0,0.0);
	for(smInt i=0;i<nbrVertices; i++)
	{
		for(j=0;j<vertTriNeighbors[i].size(); j++)
			temp += triNormals[(vertTriNeighbors[i])[j]];

		vertNormals[i]=temp;
		vertNormals[i].normalize();
		temp.setValue(0.0,0.0,0.0);
	}
}

/// \brief updates the normal of all the triangle
void smMesh::updateTriangleNormals(){

	for(smInt i=0;i<nbrTriangles; i++)
		triNormals[i] = calculateTriangleNormal(i).unit();
}

/// \brief calculates the normal of a triangle
smVec3<smFloat> smMesh::calculateTriangleNormal(smInt triNbr){

	smVec3<smFloat> v[3];
	smTriangle temp = this->triangles[triNbr];

	v[0] = this->vertices[temp.vert[0]];
	v[1] = this->vertices[temp.vert[1]];
	v[2] = this->vertices[temp.vert[2]];

	return (v[1]-v[0]).cross(v[2]-v[0]).unit();
}

/// \brief allocates vertices and related array
smBool smMesh::initVertexArrays(smInt nbr){

	if(nbr<0)
		return false;

	this->nbrVertices = nbr;
	this->vertices = new smVec3<smFloat>[nbr];
	this->origVerts = new smVec3<smFloat>[nbr];
	this->vertNormals = new smVec3<smFloat>[nbr];
	this->vertTangents = new smVec3<smFloat>[nbr];
	this->texCoord = new smTexCoord[nbr];
	return true;
}

/// \brief allocates triangle and related array
smBool smMesh::initTriangleArrays(smInt nbr){

	if(nbr<0)
		return false;

	this->nbrTriangles = nbr;

	this->triangles = new smTriangle[nbr];
	this->triNormals = new smVec3<smFloat>[nbr];
	this->triTangents = new smVec3<smFloat>[nbr];
	return true;
}

/// \brief initializes the vertex neighbors 
void smMesh::initVertexNeighbors(){

	smInt i;
	vertTriNeighbors.resize(nbrVertices);

	for(i=0; i<nbrTriangles; i++){
		vertTriNeighbors[triangles[i].vert[0]].push_back(i);
		vertTriNeighbors[triangles[i].vert[1]].push_back(i);
		vertTriNeighbors[triangles[i].vert[2]].push_back(i);
	}
}

/// \brief initializes the vertex neighbors
void smMesh::calcNeighborsVertices(){

	smInt i;
	smInt triangleIndex;
	smInt candidate[3];

	vertVertNeighbors.resize(nbrVertices);

	for(i=0; i<nbrVertices; i++){
		for(smInt j=0;j<vertTriNeighbors[i].size();j++){
			triangleIndex=vertTriNeighbors[i][j];
			candidate[0]=triangles[triangleIndex].vert[0];
			candidate[1]=triangles[triangleIndex].vert[1];
			candidate[2]=triangles[triangleIndex].vert[2];

			if(candidate[0]==i)
				candidate[0]=-1;
			if(candidate[1]==i)
				candidate[1]=-1;
			if(candidate[2]==i)
				candidate[2]=-1;

			for(smInt k=0;k<vertVertNeighbors[i].size();k++){
				if(vertVertNeighbors[i][k]==candidate[0])
					candidate[0]=-1;
				if(vertVertNeighbors[i][k]==candidate[1])
					candidate[1]=-1;
				if(vertVertNeighbors[i][k]==candidate[2])
					candidate[2]=-1;
			}

			if(candidate[0]!=-1)
				vertVertNeighbors[i].push_back(candidate[0]);
			if(candidate[1]!=-1)
				vertVertNeighbors[i].push_back(candidate[1]);
			if(candidate[2]!=-1)
				vertVertNeighbors[i].push_back(candidate[2]);

		}
	}
}

/// \brief
inline void smMesh::upadateAABB(){

	smFloat minx=smMAXFLOAT ;
	smFloat miny=smMAXFLOAT;
	smFloat minz=smMAXFLOAT;
	smFloat maxx=-smMAXFLOAT;
	smFloat maxy=-smMAXFLOAT;
	smFloat maxz=-smMAXFLOAT;

	for(smInt i=0;i<nbrVertices;i++){
		minx=SOFMIS_MIN(vertices[i].x,minx);
		miny=SOFMIS_MIN(vertices[i].y,miny);
		minz=SOFMIS_MIN(vertices[i].z,minz);

		maxx=SOFMIS_MAX(vertices[i].x,maxx);
		maxy=SOFMIS_MAX(vertices[i].y,maxy);
		maxz=SOFMIS_MAX(vertices[i].z,maxz);
	}

	aabb.aabbMin.x=minx-(maxx-minx)*SOFMIS_MESH_AABBSKINFACTOR;
	aabb.aabbMin.y=miny-(maxy-miny)*SOFMIS_MESH_AABBSKINFACTOR;
	aabb.aabbMin.z=minz-(maxz-minz)*SOFMIS_MESH_AABBSKINFACTOR;

	aabb.aabbMax.x=maxx+(maxx-minx)*SOFMIS_MESH_AABBSKINFACTOR;
	aabb.aabbMax.y=maxy+(maxy-miny)*SOFMIS_MESH_AABBSKINFACTOR;
	aabb.aabbMax.z=maxz+(maxz-minz)*SOFMIS_MESH_AABBSKINFACTOR;
}

/// \brief
void smMesh::calcEdges(){

	smInt i, j, k;
	smBool exist;
	smEdge edge;
	edges.reserve(SOFMIS_MESH_RESERVEDMAXEDGES);
	for(i=0; i<nbrVertices; i++){
		for(j=0;j<vertVertNeighbors[i].size();j++){
			if(vertVertNeighbors[i][j]>i){
					edge.vert[0]=i;
					edge.vert[1]=vertVertNeighbors[i][j];
					edges.push_back(edge);
			}
		}
	}
}

/// \brief
void smMesh::translate(smFloat p_offsetX,smFloat p_offsetY,smFloat p_offsetZ){

	for(smInt i=0; i<nbrVertices; i++){
		vertices[i].x=vertices[i].x+p_offsetX;
		vertices[i].y=vertices[i].y+p_offsetY;
		vertices[i].z=vertices[i].z+p_offsetZ;
		
		origVerts[i].x=vertices[i].x+p_offsetX;
		origVerts[i].y=vertices[i].y+p_offsetY;
		origVerts[i].z=vertices[i].z+p_offsetZ;
	}
	upadateAABB();
}

/// \brief
void smMesh::translate(smVec3<smFloat> p_offset){

	for(smInt i=0; i<nbrVertices; i++){
		vertices[i]=vertices[i]+p_offset;
		origVerts[i]=origVerts[i]+p_offset;
	}
	upadateAABB();
}

/// \brief
void smMesh::scale(smVec3<smFloat> p_scaleFactors){

	for(smInt i=0; i<nbrVertices; i++){
		vertices[i].x=vertices[i].x*p_scaleFactors.x;
		vertices[i].y=vertices[i].y*p_scaleFactors.y;
		vertices[i].z=vertices[i].z*p_scaleFactors.z;

		origVerts[i].x=origVerts[i].x*p_scaleFactors.x;
		origVerts[i].y=origVerts[i].y*p_scaleFactors.y;
		origVerts[i].z=origVerts[i].z*p_scaleFactors.z;
	}
	upadateAABB();
}

/// \brief
void smMesh::rotate(smMatrix33<smFloat> p_rot){

	for(smInt i=0; i<nbrVertices; i++){
		vertices[i]=p_rot*vertices[i];
		origVerts[i]=p_rot*origVerts[i];
		vertNormals[i]=p_rot*vertNormals[i];
	}

	for(smInt i=0; i<nbrTriangles; i++){
		triNormals[i]=p_rot*triNormals[i];
	}
	calcTriangleTangents();
	upadateAABB();
}

/// \brief
void smMesh::draw(smDrawParam p_params){

	smViewer *viewer=p_params.rendererObject;
	if(viewer->renderStage==SMRENDERSTAGE_SHADOWPASS&&p_params.caller->renderDetail.castShadow==false)
		return;
smGLRenderer::drawSurfaceMeshTriangles(this,&p_params.caller->renderDetail,p_params);
	if(p_params.caller->renderDetail.renderType&SOFMIS_RENDER_NORMALS)
		smGLRenderer::drawNormals(this,p_params.caller->renderDetail.normalColor);
}

/// \brief
void smMesh::updateTriangleAABB(){

	smAABB tempAABB;
	tempAABB.aabbMin.x=FLT_MAX;
	tempAABB.aabbMin.y=FLT_MAX;
	tempAABB.aabbMin.z=FLT_MAX;

	tempAABB.aabbMax.x=-FLT_MAX;
	tempAABB.aabbMax.y=-FLT_MAX;
	tempAABB.aabbMax.z=-FLT_MAX;

	for(smInt i=0;i<nbrTriangles;i++){
		// min
		triAABBs[i].aabbMin.x = SOFMIS_MIN(vertices[triangles[i].vert[0]].x, vertices[triangles[i].vert[1]].x); 
		triAABBs[i].aabbMin.x = SOFMIS_MIN(triAABBs[i].aabbMin.x ,   vertices[triangles[i].vert[2]].x);
		tempAABB.aabbMin.x = SOFMIS_MIN(tempAABB.aabbMin.x,triAABBs[i].aabbMin.x);

		triAABBs[i].aabbMin.y = SOFMIS_MIN(vertices[triangles[i].vert[0]].y, vertices[triangles[i].vert[1]].y);
		triAABBs[i].aabbMin.y = SOFMIS_MIN(triAABBs[i].aabbMin.y ,   vertices[triangles[i].vert[2]].y);
		tempAABB.aabbMin.y = SOFMIS_MIN(tempAABB.aabbMin.y,triAABBs[i].aabbMin.y);

		triAABBs[i].aabbMin.z = SOFMIS_MIN(vertices[triangles[i].vert[0]].z, vertices[triangles[i].vert[1]].z);
		triAABBs[i].aabbMin.z = SOFMIS_MIN(triAABBs[i].aabbMin.z ,   vertices[triangles[i].vert[2]].z);
		tempAABB.aabbMin.z = SOFMIS_MIN(tempAABB.aabbMin.z,triAABBs[i].aabbMin.z);

		//max
		triAABBs[i].aabbMax.x = SOFMIS_MAX(vertices[triangles[i].vert[0]].x, vertices[triangles[i].vert[1]].x);
		triAABBs[i].aabbMax.x = SOFMIS_MAX(triAABBs[i].aabbMax.x ,   vertices[triangles[i].vert[2]].x);
		tempAABB.aabbMax.x = SOFMIS_MAX(tempAABB.aabbMax.x,triAABBs[i].aabbMax.x );

		triAABBs[i].aabbMax.y = SOFMIS_MAX(vertices[triangles[i].vert[0]].y, vertices[triangles[i].vert[1]].y);
		triAABBs[i].aabbMax.y = SOFMIS_MAX(triAABBs[i].aabbMax.y ,   vertices[triangles[i].vert[2]].y);
		tempAABB.aabbMax.y = SOFMIS_MAX(tempAABB.aabbMax.y, triAABBs[i].aabbMax.y);

		triAABBs[i].aabbMax.z = SOFMIS_MAX(triAABBs[i].aabbMax.z,    vertices[triangles[i].vert[2]].z);
		tempAABB.aabbMax.z = SOFMIS_MAX(tempAABB.aabbMax.z, triAABBs[i].aabbMax.z);
	}
	aabb=tempAABB;
}

/// \brief
void smMesh::checkCorrectWinding(){

	smEdge  edge1;
	smEdge  edge2;
	smEdge  edge3;
	smInt x[3];
	smInt p[3];
	smInt edgeMatch=0;

	for(smInt i=0;i<nbrTriangles;i++){
		edgeMatch=0;
		x[0]=triangles[i].vert[0];
		x[1]=triangles[i].vert[1];
		x[2]=triangles[i].vert[2];

		for(smInt j=0;j<nbrTriangles;j++){
			if(j==i)
				continue;
			p[0]=triangles[j].vert[0];
			p[1]=triangles[j].vert[1];
			p[2]=triangles[j].vert[2];

			if(x[0]==p[0]&&x[1]==p[1])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			if(x[0]==p[1]&&x[1]==p[2])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			if(x[0]==p[2]&&x[1]==p[0])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			
			if(x[1]==p[0]&&x[2]==p[1])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			if(x[1]==p[1]&&x[2]==p[2])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			if(x[1]==p[2]&&x[2]==p[0])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;

			if(x[2]==p[0]&&x[0]==p[1])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			if(x[2]==p[1]&&x[0]==p[2])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
			if(x[2]==p[2]&&x[0]==p[0])	cout<<"Wrong Winding Triangles:"<<i<<","<<j<<endl;
		}
	}
}

/// \brief
void smLineMesh::draw(smDrawParam p_params){

	smViewer *viewer=p_params.rendererObject;
	smGLRenderer::drawLineMesh(this,&p_params.caller->renderDetail);
	if(p_params.caller->renderDetail.debugDraw){
		smGLRenderer::draw(this->aabb);
		for(smInt i=0;i<nbrEdges;i++){
		
			smGLRenderer::draw(this->edgeAABBs[i]);
			glPushMatrix();
			glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,smColor::colorYellow.toGLColor());
				glTranslatef(edgeAABBs[i].aabbMin.x,edgeAABBs[i].aabbMin.y,edgeAABBs[i].aabbMin.z);
				glutSolidSphere(0.2,15.0,15.0);
			glPopMatrix();
			glPushMatrix();
				glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,smColor::colorRed.toGLColor());
				glTranslatef(edgeAABBs[i].aabbMax.x,edgeAABBs[i].aabbMax.y,edgeAABBs[i].aabbMax.z);
				glutSolidSphere(0.2,15.0,15.0);
			glPopMatrix();
		}
	}
}
