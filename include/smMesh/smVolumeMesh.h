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
    \brief	    This Module is for Volume Mesh class. This is derived from Mesh class

*****************************************************
*/
#ifndef SMVOLUMEMESH_H
#define SMVOLUMEMESH_H

#include "smMesh/smMesh.h"

///brief: this class is derived from generic Mesh class. Tetrahedron are building blocks of this volume mesh.
///It also retains the surface triangle structure for rendering and collision detection purposes.
///This surface triangle structure might be extracted from the volume mesh while loading
class smVolumeMesh:public smMesh{
 
public:
	///push smMesh class specific errors here
    smErrorLog *log_VM;

    smInt  nbrNodes;
	smVec3<float> *nodes;
	
    smInt  nbrTetra;
	smTetrahedra *tetra;
	
	smInt *surfaceNodeIndex;

	smBool *fixed;


public:
	smVolumeMesh(){
	 nbrNodes=0;
	 nbrTetra=0;
	 tetra=NULL;
	 surfaceNodeIndex=NULL;
	 fixed=NULL;

	
	}

	smVolumeMesh(smMeshType meshtype,smErrorLog *log);
	~smVolumeMesh();

	void GenerateTetra(smChar *fileName);

	smBool LoadTetra(const smChar *fileName);

	smBool getSurface(const smChar *fileName);

	smBool readBC(const smChar *fileName);

	void initSurface();

	void copySurface();
	void translateVolumeMesh(smVec3<smFloat> p_translate);
	void scaleVolumeMesh(smVec3<smFloat> p_scale);

	void rotVolumeMesh(smMatrix33<smFloat> p_rot);

	smBool loadMesh(smChar *fileName,smMeshFileType fileType);


 

    
};

#endif