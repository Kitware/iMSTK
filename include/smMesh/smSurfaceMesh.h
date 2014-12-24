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
class smSurfaceMesh: public smMesh
{

public:
    /// \brief push smMesh class specific errors here
    smErrorLog *log_SF;

    /// \brief constructor
    smSurfaceMesh(smMeshType meshtype, smErrorLog *log);

    /// \brief destructor
    virtual ~smSurfaceMesh();

    /// \brief load the surface mesh
    smBool loadMesh(smChar *fileName, smMeshFileType fileType);

    /// \brief load the surface mesh using in-house code
    smBool loadMeshLegacy(smChar *fileName, smMeshFileType fileType);

    /// \brief load the surface mesh from 3ds format
    smBool Load3dsMesh(smChar *fileName);

    /// \brief load surface meshes using assimp library
    smBool LoadMeshAssimp(const smChar *fileName);
};

#endif
