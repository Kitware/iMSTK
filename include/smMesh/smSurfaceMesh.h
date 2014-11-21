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
	\brief	    This Module is for Surface Mesh class. This is derived from Mesh class

*****************************************************
*/

#ifndef SMSURFACEMESH_H
#define SMSURFACEMESH_H

#include "smMesh/smMesh.h"


///brief: this is the Surface Mesh class derived from generic Mesh class. 
class smSurfaceMesh:public smMesh{
public:
	///push smMesh class specific errors here
	smErrorLog *log_SF;

	smSurfaceMesh(smMeshType meshtype,smErrorLog *log);
	virtual ~smSurfaceMesh();

	smBool loadMesh(smChar *fileName,smMeshFileType fileType);
    smBool loadMeshLegacy(smChar *fileName,smMeshFileType fileType);
	smBool Load3dsMesh(smChar *fileName);
	smBool LoadMeshAssimp(const smChar *fileName);
};

#endif
