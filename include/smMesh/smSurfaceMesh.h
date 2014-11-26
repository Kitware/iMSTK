/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMSURFACEMESH_H
#define SMSURFACEMESH_H

#include "smMesh/smMesh.h"

/// \brief this is the Surface Mesh class derived from generic Mesh class. 
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
