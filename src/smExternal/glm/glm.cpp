/*    
*  GLM library.  Wavefront .obj file format reader/writer/manipulator.
*
*  Written by Nate Robins, 1997.
*  email: ndr@pobox.com
*  www: http://www.pobox.com/~ndr
*/

 
/* includes */
#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "glm.h"





/* defines */
#define T(x) model->triangles[(x)]


/* enums */ 
enum { X, Y, Z, W };			/* elements of a vertex */


/* typedefs */

/* _GLMnode: general purpose node
*/
typedef struct _GLMnode {
	GLuint           index;
	GLboolean        averaged;
	struct _GLMnode* next;
} GLMnode;






GLMgroup*
_glmFindGroup(GLMmodel* model, char* name)
{
	GLMgroup* group;

	assert(model);

	group = model->groups;
	while(group) {
		if (!strcmp(name, group->name))
			break;
		group = group->next;
	}

	return group;
}

/* _glmAddGroup: Add a group to the model
*/
GLMgroup*
_glmAddGroup(GLMmodel* model, char* name)
{
	GLMgroup* group;

	group = _glmFindGroup(model, name);
	if (!group) {
		group = (GLMgroup*)malloc(sizeof(GLMgroup));
		group->name = strdup(name);
		group->material = 0;
		group->numtriangles = 0;
		group->triangles = NULL;
		group->next = model->groups;
		model->groups = group;
		model->numgroups++;
	}

	return group;
}

/* _glmFindGroup: Find a material in the model
*/
GLuint
_glmFindMaterial(GLMmodel* model, char* name)
{
	GLuint i;

	for (i = 0; i < model->nummaterials; i++) {
		if (model->materials[i].name!=NULL&&!strcmp(model->materials[i].name, name))
			goto found;
	}

	/* didn't find the name, so set it as the default material */
	//printf("_glmFindMaterial():  can't find material \"%s\".\n", name);
	i = 0;

found:
	return i;
}


/* _glmDirName: return the directory given a path
*
* path - filesystem path
*
* The return value should be free'd.
*/
static char*
_glmDirName(char* path)
{
	char* dir;
	char* s;

	dir = strdup(path);

	s = strrchr(dir, '\\');
	if (s)
		s[1] = '\0';
	else
		dir[0] = '\0';

	return dir;
}





/* _glmReadMTL: read a wavefront material library file
*
* model - properly initialized GLMmodel structure
* name  - name of the material library
*/
static GLvoid
_glmReadMTL(GLMmodel* model, char* name)
{
	FILE* file;
	char* dir;
	char* filename;
	char  buf[128];
	GLuint nummaterials, i;
	printf("MTL NAME =%s\n",name);
	dir = _glmDirName(model->pathname);

	filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 1));
	strcpy(filename, dir);
	strcat(filename, name);
	free(dir);
	printf("MODEL PATH=%s\n",model->pathname);
	printf("MODEL PATH=%s\n",dir);
	printf("MTL filename=%s\n",filename);

	/* open the file */
	file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "_glmReadMTL() failed: can't open material file \"%s\".\n",
			filename);
		exit(1);
	}
	free(filename);

	/* count the number of materials in the file */
	nummaterials = 1;
	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
case '#':				/* comment */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
case 'n':				/* newmtl */
	fgets(buf, sizeof(buf), file);
	nummaterials++;
	sscanf(buf, "%s %s", buf, buf);
	break;
default:
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
		}
	}

	rewind(file);

	/* allocate memory for the materials */
	model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
	model->nummaterials = nummaterials;


	/* set the default material */
	for (i = 0; i < nummaterials; i++) {
		model->materials[i].name = NULL;
		model->materials[i].shininess = 0;
		model->materials[i].diffuse[0] = 0.8;
		model->materials[i].diffuse[1] = 0.8;
		model->materials[i].diffuse[2] = 0.8;
		model->materials[i].diffuse[3] = 1.0;
		model->materials[i].ambient[0] = 0.2;
		model->materials[i].ambient[1] = 0.2;
		model->materials[i].ambient[2] = 0.2;
		model->materials[i].ambient[3] = 1.0;
		model->materials[i].specular[0] = 0.0;
		model->materials[i].specular[1] = 0.0;
		model->materials[i].specular[2] = 0.0;
		model->materials[i].specular[3] = 1.0;
		
		model->materials[i].emmissive[0] = 0.0;
		model->materials[i].emmissive[1] = 0.0;
		model->materials[i].emmissive[2] = 0.0;
		model->materials[i].emmissive[3] = 1.0;
		
		memset(model->materials[i].textureFile,'\0',255);
		//model->materials[i].textureId=0;

	}
	model->materials[0].name = strdup("default");

	/* now, read in the data */
	nummaterials = 0;
	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
case '#':				/* comment */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
case 'n':				/* newmtl */
	fgets(buf, sizeof(buf), file);
	sscanf(buf, "%s %s", buf, buf);
	nummaterials++;
	model->materials[nummaterials].name = strdup(buf);
	break;
case 'N':
	fscanf(file, "%f", &model->materials[nummaterials].shininess);
	/* wavefront shininess is from [0, 1000], so scale for OpenGL */
	model->materials[nummaterials].shininess /= 1000.0;
	model->materials[nummaterials].shininess *= 128.0;
	break;

case 'd':
	fgets(buf, sizeof(buf), file);
	sscanf(buf, "%f",&model->materials[nummaterials].diffuse[3]);
	break;
case 'm':
	fgets(buf,sizeof(buf),file);
	
	if(strlen(buf)<=2)
		break;
	//fscanf(file, "%s", buf);
	sscanf(buf, "%s", buf);
	if(buf[1]!='-'&&buf[2]!='s')
	{

		sscanf(buf,"%s",model->materials[nummaterials].textureFile);
		//model->numberTexturesFiles++;
		printf("%s\n",model->materials[nummaterials].textureFile);
	}
	else{
		/*fscanf(file, "%s", buf);
		fscanf(file, "%s", buf);
		fscanf(file, "%s", buf);*/


		sscanf(buf,"%s",model->materials[nummaterials].textureFile);
		printf("%s\n",model->materials[nummaterials].textureFile);



	}
	break;

case 'K':
	switch(buf[1]) {
case 'd':
	fscanf(file, "%f %f %f",
		&model->materials[nummaterials].diffuse[0],
		&model->materials[nummaterials].diffuse[1],
		&model->materials[nummaterials].diffuse[2]);
	break;
case 's':
	fscanf(file, "%f %f %f",
		&model->materials[nummaterials].specular[0],
		&model->materials[nummaterials].specular[1],
		&model->materials[nummaterials].specular[2]);
	break;
case 'a':
	fscanf(file, "%f %f %f",
		&model->materials[nummaterials].ambient[0],
		&model->materials[nummaterials].ambient[1],
		&model->materials[nummaterials].ambient[2]);
	break;
default:
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
	}
	break;
default:
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
		}
	}

	fclose(file);

}


/* _glmFirstPass: first pass at a Wavefront OBJ file that gets all the
* statistics of the model (such as #vertices, #normals, etc)
*
* model - properly initialized GLMmodel structure
* file  - (fopen'd) file descriptor 
*/
static GLvoid
_glmFirstPass(GLMmodel* model, FILE* file) 
{
	GLuint    numvertices;		/* number of vertices in model */
	GLuint    numnormals;			/* number of normals in model */
	GLuint    numtexcoords;		/* number of texcoords in model */
	GLuint    numtriangles;		/* number of triangles in model */
	GLMgroup* group;			/* current group */
	unsigned  v, n, t;
	char      buf[128];

	/* make a default group */
	group = _glmAddGroup(model, "default");

	numvertices = numnormals = numtexcoords = numtriangles = 0;
	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
case '#':				/* comment */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
case 'v':				/* v, vn, vt */
	switch(buf[1]) {
case '\0':			/* vertex */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	numvertices++;
	break;
case 'n':				/* normal */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	numnormals++;
	break;
case 't':				/* texcoord */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	numtexcoords++;
	break;
default:
	printf("_glmFirstPass(): Unknown token \"%s\".\n", buf);
	exit(1);
	break;
	}
	break;
case 'm':
	fgets(buf, sizeof(buf), file);
	sscanf(buf, "%s %s", buf, buf);
	model->mtllibname = strdup(buf);
	//_glmReadMTL(model, buf);
	break;
case 'u':
	
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
case 'g':				/* group */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	sscanf(buf, "%s", buf);
	group = _glmAddGroup(model, buf);
	break;
case 'f':				/* face */
	v = n = t = 0;
	fscanf(file, "%s", buf);
	/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
	if (strstr(buf, "//")) {
		/* v//n */
		sscanf(buf, "%d//%d", &v, &n);
		fscanf(file, "%d//%d", &v, &n);
		fscanf(file, "%d//%d", &v, &n);
		numtriangles++;
		group->numtriangles++;
		while(fscanf(file, "%d//%d", &v, &n) > 0) {
			numtriangles++;
			group->numtriangles++;
		}
	} else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
		/* v/t/n */
		fscanf(file, "%d/%d/%d", &v, &t, &n);
		fscanf(file, "%d/%d/%d", &v, &t, &n);
		numtriangles++;
		group->numtriangles++;
		while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
			numtriangles++;
			group->numtriangles++;
		}
	} else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
		/* v/t */
		fscanf(file, "%d/%d", &v, &t);
		fscanf(file, "%d/%d", &v, &t);
		numtriangles++;
		group->numtriangles++;
		while(fscanf(file, "%d/%d", &v, &t) > 0) {
			numtriangles++;
			group->numtriangles++;
		}
	} else {
		/* v */
		fscanf(file, "%d", &v);
		fscanf(file, "%d", &v);
		numtriangles++;
		group->numtriangles++;
		while(fscanf(file, "%d", &v) > 0) {
			numtriangles++;
			group->numtriangles++;
		}
	}
	break;

default:
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
		}
	}

#if 0
	/* announce the model statistics */
	printf(" Vertices: %d\n", numvertices);
	printf(" Normals: %d\n", numnormals);
	printf(" Texcoords: %d\n", numtexcoords);
	printf(" Triangles: %d\n", numtriangles);
	printf(" Groups: %d\n", model->numgroups);
#endif

	/* set the stats in the model structure */
	model->numvertices  = numvertices;
	model->numnormals   = numnormals;
	model->numtexcoords = numtexcoords;
	model->numtriangles = numtriangles;

	/* allocate memory for the triangles in each group */
	group = model->groups;
	while(group) {
		group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
		group->numtriangles = 0;
		group = group->next;
	}
}

/* _glmSecondPass: second pass at a Wavefront OBJ file that gets all
* the data.
*
* model - properly initialized GLMmodel structure
* file  - (fopen'd) file descriptor 
*/
static GLvoid
_glmSecondPass(GLMmodel* model, FILE* file) 
{
	GLuint    numvertices;		/* number of vertices in model */
	GLuint    numnormals;			/* number of normals in model */
	GLuint    numtexcoords;		/* number of texcoords in model */
	GLuint    numtriangles;		/* number of triangles in model */
	GLfloat*  vertices;			/* array of vertices  */
	GLfloat*  normals;			/* array of normals */
	GLfloat*  texcoords;			/* array of texture coordinates */
	GLMgroup* group;			/* current group pointer */
	GLuint    material;			/* current material */
	GLuint    v, n, t;
	char      buf[128];

	/* set the pointer shortcuts */
	vertices     = model->vertices;
	normals      = model->normals;
	texcoords    = model->texcoords;
	group        = model->groups;

	/* on the second pass through the file, read all the data into the
	allocated arrays */
	numvertices = numnormals = numtexcoords = 1;
	numtriangles = 0;
	material = 0;
	while(fscanf(file, "%s", buf) != EOF) {
		switch(buf[0]) {
case '#':				/* comment */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
case 'v':				/* v, vn, vt */
	switch(buf[1]) {
case '\0':			/* vertex */
	fscanf(file, "%f %f %f", 
		&vertices[3 * numvertices + X], 
		&vertices[3 * numvertices + Y], 
		&vertices[3 * numvertices + Z]);
	numvertices++;
	break;
case 'n':				/* normal */
	fscanf(file, "%f %f %f", 
		&normals[3 * numnormals + X],
		&normals[3 * numnormals + Y], 
		&normals[3 * numnormals + Z]);
	numnormals++;
	break;
case 't':				/* texcoord */
	fscanf(file, "%f %f", 
		&texcoords[2 * numtexcoords + X],
		&texcoords[2 * numtexcoords + Y]);
	numtexcoords++;
	break;
	}
	break;
case 'u':
	fgets(buf, sizeof(buf), file);
	sscanf(buf, "%s %s", buf, buf);
	group->material = material = _glmFindMaterial(model, buf);

	break;
case 'g':				/* group */
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	sscanf(buf, "%s", buf);
	group = _glmFindGroup(model, buf);
	group->material = material;
	break;
case 'f':				/* face */
	v = n = t = 0;
	fscanf(file, "%s", buf);
	
	/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
	if (strstr(buf, "//")) {
		/* v//n */
		sscanf(buf, "%d//%d", &v, &n);
		T(numtriangles).vindices[0] = v;
		T(numtriangles).nindices[0] = n;
		fscanf(file, "%d//%d", &v, &n);
		T(numtriangles).vindices[1] = v;
		T(numtriangles).nindices[1] = n;
		fscanf(file, "%d//%d", &v, &n);
		T(numtriangles).vindices[2] = v; 
		T(numtriangles).nindices[2] = n;
		T(numtriangles).textureEnable=0;
		T(numtriangles).materialId=material;//by tansel dikkaaat
		group->triangles[group->numtriangles++] = numtriangles;
		numtriangles++;

		while(fscanf(file, "%d//%d", &v, &n) > 0) {
			T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
			T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
			T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
			T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
			T(numtriangles).vindices[2] = v;
			T(numtriangles).nindices[2] = n;
			T(numtriangles).textureEnable=0;
			T(numtriangles).materialId=material;//by tansel dikkaaat
			group->triangles[group->numtriangles++] = numtriangles;
			numtriangles++;
		}
	} else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {

		model->OBJtype=1;

		/* v/t/n */
		T(numtriangles).vindices[0] = v;
		T(numtriangles).tindices[0] = t;
		T(numtriangles).nindices[0] = n;
		fscanf(file, "%d/%d/%d", &v, &t, &n);
		T(numtriangles).vindices[1] = v;
		T(numtriangles).tindices[1] = t;
		T(numtriangles).nindices[1] = n;
		fscanf(file, "%d/%d/%d", &v, &t, &n);
		T(numtriangles).vindices[2] = v;
		T(numtriangles).tindices[2] = t;
		T(numtriangles).nindices[2] = n;
		T(numtriangles).textureEnable=1;
		T(numtriangles).materialId=material;//by tansel dikkaaat
		group->triangles[group->numtriangles++] = numtriangles;
		numtriangles++;
		while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
			T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
			T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
			T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
			T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
			T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
			T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
			T(numtriangles).vindices[2] = v;
			T(numtriangles).tindices[2] = t;
			T(numtriangles).nindices[2] = n;
			T(numtriangles).textureEnable=1;
			T(numtriangles).materialId=material;//by tansel dikkaaat
			group->triangles[group->numtriangles++] = numtriangles;
			numtriangles++;
		}
	} else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
		/* v/t */
		T(numtriangles).vindices[0] = v;
		T(numtriangles).tindices[0] = t;

		fscanf(file, "%d/%d", &v, &t);
		T(numtriangles).vindices[1] = v;
		T(numtriangles).tindices[1] = t;
		fscanf(file, "%d/%d", &v, &t);
		T(numtriangles).vindices[2] = v;
		T(numtriangles).tindices[2] = t;
		T(numtriangles).textureEnable=1;
		T(numtriangles).materialId=material;//by tansel dikkaaat
		group->triangles[group->numtriangles++] = numtriangles;
		numtriangles++;
		while(fscanf(file, "%d/%d", &v, &t) > 0) {
			T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
			T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
			T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
			T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
			T(numtriangles).vindices[2] = v;
			T(numtriangles).tindices[2] = t;
			T(numtriangles).textureEnable=1;
			T(numtriangles).materialId=material;//by tansel dikkaaat
			group->triangles[group->numtriangles++] = numtriangles;
			numtriangles++;
		}
	} else {
		/* v */
		sscanf(buf, "%d", &v);
		T(numtriangles).vindices[0] = v;
		T(numtriangles).textureEnable=0;
		T(numtriangles).materialId=material;//by tansel dikkaaat
		fscanf(file, "%d", &v);
		T(numtriangles).vindices[1] = v;
		fscanf(file, "%d", &v);
		T(numtriangles).vindices[2] = v;
		group->triangles[group->numtriangles++] = numtriangles;
		numtriangles++;
		while(fscanf(file, "%d", &v) > 0) {
			T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
			T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
			T(numtriangles).vindices[2] = v;
			T(numtriangles).textureEnable=0;
			T(numtriangles).materialId=material;//by tansel dikkaaat
			group->triangles[group->numtriangles++] = numtriangles;
			numtriangles++;
		}
	}
	break;

default:
	/* eat up rest of line */
	fgets(buf, sizeof(buf), file);
	break;
		}
	}
fclose(file);

#if 0
	/* announce the memory requirements */
	printf(" Memory: %d bytes\n",
		numvertices  * 3*sizeof(GLfloat) +
		numnormals   * 3*sizeof(GLfloat) * (numnormals ? 1 : 0) +
		numtexcoords * 3*sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
		numtriangles * sizeof(GLMtriangle));
#endif
}








GLvoid
glmDelete(GLMmodel* model)
{
	GLMgroup* group;
	GLuint i;

	assert(model);

	if (model->pathname)   free(model->pathname);
	if (model->mtllibname) free(model->mtllibname);
	if (model->vertices)   free(model->vertices);
	if (model->normals)    free(model->normals);
	if (model->texcoords)  free(model->texcoords);
	if (model->facetnorms) free(model->facetnorms);
	if (model->triangles)  free(model->triangles);
	if (model->materials) {
		for (i = 0; i < model->nummaterials; i++)
			free(model->materials[i].name);
	}
	free(model->materials);
	while(model->groups) {
		group = model->groups;
		model->groups = model->groups->next;
		free(group->name);
		free(group->triangles);
		free(group);
	}

	free(model);
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
* Returns a pointer to the created object which should be free'd with
* glmDelete().
*
* filename - name of the file containing the Wavefront .OBJ format data.  
*/
GLMmodel* 
glmReadOBJ(char* filename)
{
	GLMmodel* model;
	FILE*     file;

	/* open the file */
	file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "glmReadOBJ() failed: can't open data file \"%s\".\n",
			filename);
		exit(1);
	}

#if 0
	/* announce the model name */
	printf("Model: %s\n", filename);
#endif

	/* allocate a new model */
	model = (GLMmodel*)malloc(sizeof(GLMmodel));
	model->pathname      = strdup(filename);
	model->mtllibname    = NULL;
	model->numvertices   = 0;
	model->vertices      = NULL;
	model->numnormals    = 0;
	model->normals       = NULL;
	model->numtexcoords  = 0;
	model->texcoords     = NULL;
	model->numfacetnorms = 0;
	model->facetnorms    = NULL;
	model->numtriangles  = 0;
	model->triangles     = NULL;
	model->nummaterials  = 0;
	model->materials     = NULL;
	model->numgroups     = 0;
	model->groups        = NULL;
	model->position[0]   = 0.0;
	model->position[1]   = 0.0;
	model->position[2]   = 0.0;
	model->OBJtype=1;

	/* make a first pass through the file to get a count of the number
	of vertices, normals, texcoords & triangles */
	_glmFirstPass(model, file);

	/* allocate memory */
	model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numvertices + 1));
	model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
		model->numtriangles);
	if (model->numnormals) {
		model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
			3 * (model->numnormals + 1));
	}
	if (model->numtexcoords) {
		model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
			2 * (model->numtexcoords + 1));
	}

	/* rewind to beginning of file and read in the data this pass */
	rewind(file);

	_glmSecondPass(model, file);

	/* close the file */
	fclose(file);

	return model;
}



#if 0
/* normals */
if (model->numnormals) {
	numvectors = model->numnormals;
	vectors    = model->normals;
	copies = _glmOptimizeVectors(vectors, &numvectors);

	printf("glmOptimize(): %d redundant normals.\n", 
		model->numnormals - numvectors);

	for (i = 0; i < model->numtriangles; i++) {
		T(i).nindices[0] = (GLuint)vectors[3 * T(i).nindices[0] + 0];
		T(i).nindices[1] = (GLuint)vectors[3 * T(i).nindices[1] + 0];
		T(i).nindices[2] = (GLuint)vectors[3 * T(i).nindices[2] + 0];
	}

	/* free space for old normals */
	free(vectors);

	/* allocate space for the new normals */
	model->numnormals = numvectors;
	model->normals = (GLfloat*)malloc(sizeof(GLfloat) * 
		3 * (model->numnormals + 1));

	/* copy the optimized vertices into the actual vertex list */
	for (i = 1; i <= model->numnormals; i++) {
		model->normals[3 * i + 0] = copies[3 * i + 0];
		model->normals[3 * i + 1] = copies[3 * i + 1];
		model->normals[3 * i + 2] = copies[3 * i + 2];
	}

	free(copies);
}

/* texcoords */
if (model->numtexcoords) {
	numvectors = model->numtexcoords;
	vectors    = model->texcoords;
	copies = _glmOptimizeVectors(vectors, &numvectors);

	printf("glmOptimize(): %d redundant texcoords.\n", 
		model->numtexcoords - numvectors);

	for (i = 0; i < model->numtriangles; i++) {
		for (j = 0; j < 3; j++) {
			T(i).tindices[j] = (GLuint)vectors[3 * T(i).tindices[j] + 0];
		}
	}

	/* free space for old texcoords */
	free(vectors);

	/* allocate space for the new texcoords */
	model->numtexcoords = numvectors;
	model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 
		2 * (model->numtexcoords + 1));

	/* copy the optimized vertices into the actual vertex list */
	for (i = 1; i <= model->numtexcoords; i++) {
		model->texcoords[2 * i + 0] = copies[2 * i + 0];
		model->texcoords[2 * i + 1] = copies[2 * i + 1];
	}

	free(copies);
}
#endif

#if 0
/* look for unused vertices */
/* look for unused normals */
/* look for unused texcoords */
for (i = 1; i <= model->numvertices; i++) {
	for (j = 0; j < model->numtriangles; i++) {
		if (T(j).vindices[0] == i || 
			T(j).vindices[1] == i || 
			T(j).vindices[1] == i)
			break;
	}
}
#endif


static GLvoid
_glmWriteMTL(GLMmodel* model, char* modelpath, char* mtllibname)
{
	FILE* file;
	char* dir;
	char* filename;
	GLMmaterial* material;
	GLuint i;

	dir = _glmDirName(modelpath);
	filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(mtllibname)+1));
	strcpy(filename, dir);
	strcat(filename, mtllibname);
	free(dir);

	/* open the file */
	file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "_glmWriteMTL() failed: can't open file \"%s\".\n",
			filename);
		exit(1);
	}
	free(filename);

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront MTL generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library copyright (C) 1997 by Nate Robins\n");
	fprintf(file, "#  email: ndr@pobox.com\n");
	fprintf(file, "#  www:   http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n\n");

	for (i = 0; i < model->nummaterials; i++) {
		material = &model->materials[i];
		fprintf(file, "newmtl %s\n", material->name);
		fprintf(file, "Ka %f %f %f\n", 
			material->ambient[0], material->ambient[1], material->ambient[2]);
		fprintf(file, "Kd %f %f %f\n", 
			material->diffuse[0], material->diffuse[1], material->diffuse[2]);
		fprintf(file, "Ks %f %f %f\n", 
			material->specular[0],material->specular[1],material->specular[2]);
		fprintf(file, "Ns %f\n", material->shininess);
		fprintf(file, "\n");
	}
}



/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
* a file.
*
* model    - initialized GLMmodel structure
* filename - name of the file to write the Wavefront .OBJ format data to
* mode     - a bitwise or of values describing what is written to the file
*            GLM_NONE     -  render with only vertices
*            GLM_FLAT     -  render with facet normals
*            GLM_SMOOTH   -  render with vertex normals
*            GLM_TEXTURE  -  render with texture coords
*            GLM_COLOR    -  render with colors (color material)
*            GLM_MATERIAL -  render with materials
*            GLM_COLOR and GLM_MATERIAL should not both be specified.  
*            GLM_FLAT and GLM_SMOOTH should not both be specified.  
*/
GLvoid
glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode)
{
	GLuint    i;
	FILE*     file;
	GLMgroup* group;

	assert(model);

	/* do a bit of warning */
	if (mode & GLM_FLAT && !model->facetnorms) {
		printf("glmWriteOBJ() warning: flat normal output requested "
			"with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !model->normals) {
		printf("glmWriteOBJ() warning: smooth normal output requested "
			"with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !model->texcoords) {
		printf("glmWriteOBJ() warning: texture coordinate output requested "
			"with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		printf("glmWriteOBJ() warning: flat normal output requested "
			"and smooth normal output requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}

	/* open the file */
	file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "glmWriteOBJ() failed: can't open file \"%s\" to write.\n",
			filename);
		exit(1);
	}

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library copyright (C) 1997 by Nate Robins\n");
	fprintf(file, "#  email: ndr@pobox.com\n");
	fprintf(file, "#  www:   http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n");

	if (mode & GLM_MATERIAL && model->mtllibname) {
		fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
		_glmWriteMTL(model, filename, model->mtllibname);
	}

	/* spit out the vertices */
	fprintf(file, "\n");
	fprintf(file, "# %d vertices\n", model->numvertices);
	for (i = 1; i <= model->numvertices; i++) {
		fprintf(file, "v %f %f %f\n", 
			model->vertices[3 * i + 0],
			model->vertices[3 * i + 1],
			model->vertices[3 * i + 2]);
	}

	/* spit out the smooth/flat normals */
	if (mode & GLM_SMOOTH) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", model->numnormals);
		for (i = 1; i <= model->numnormals; i++) {
			fprintf(file, "vn %f %f %f\n", 
				model->normals[3 * i + 0],
				model->normals[3 * i + 1],
				model->normals[3 * i + 2]);
		}
	} else if (mode & GLM_FLAT) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", model->numfacetnorms);
		for (i = 1; i <= model->numnormals; i++) {
			fprintf(file, "vn %f %f %f\n", 
				model->facetnorms[3 * i + 0],
				model->facetnorms[3 * i + 1],
				model->facetnorms[3 * i + 2]);
		}
	}

	/* spit out the texture coordinates */
	if (mode & GLM_TEXTURE) {
		fprintf(file, "\n");
		fprintf(file, "# %d texcoords\n", model->texcoords);
		for (i = 1; i <= model->numtexcoords; i++) {
			fprintf(file, "vt %f %f\n", 
				model->texcoords[2 * i + 0],
				model->texcoords[2 * i + 1]);
		}
	}

	fprintf(file, "\n");
	fprintf(file, "# %d groups\n", model->numgroups);
	fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
	fprintf(file, "\n");

	group = model->groups;
	while(group) {
		fprintf(file, "g %s\n", group->name);
		if (mode & GLM_MATERIAL)
			fprintf(file, "usemtl %s\n", model->materials[group->material].name);
		for (i = 0; i < group->numtriangles; i++) {
			if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					T(group->triangles[i]).vindices[0], 
					T(group->triangles[i]).nindices[0], 
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).nindices[2],
					T(group->triangles[i]).tindices[2]);
			} else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex);
			} else if (mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).tindices[2]);
			} else if (mode & GLM_SMOOTH) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).vindices[2], 
					T(group->triangles[i]).nindices[2]);
			} else if (mode & GLM_FLAT) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
					T(group->triangles[i]).vindices[0], 
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex);
			} else {
				fprintf(file, "f %d %d %d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).vindices[2]);
			}
		}
		fprintf(file, "\n");
		group = group->next;
	}

	fclose(file);
}

