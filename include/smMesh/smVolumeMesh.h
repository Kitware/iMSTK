/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMVOLUMEMESH_H
#define SMVOLUMEMESH_H

#include "smMesh/smMesh.h"

/// \brief this class is derived from generic Mesh class. Tetrahedron are building blocks of this volume mesh.
///  It also retains the surface triangle structure for rendering and collision detection purposes.
///  This surface triangle structure might be extracted from the volume mesh while loading
class smVolumeMesh: public smMesh
{

public:
    /// push smMesh class specific errors here
    smErrorLog *log_VM; ///< log the errors with volume mesh class
    smInt nbrNodes; ///< total number of nodes of the volume mesh
    smVec3<float> *nodes; ///< data of nodal co-ordinates
    smInt nbrTetra; ///< number of tetrahedra
    smTetrahedra *tetra; ///< tetrahedra data
    smInt *surfaceNodeIndex; ///<
    smBool *fixed; ///< indicates if the node is fixed or not

    /// \brief constructor
    smVolumeMesh()
    {
        nbrNodes = 0;
        nbrTetra = 0;
        tetra = NULL;
        surfaceNodeIndex = NULL;
        fixed = NULL;
    }

    /// \brief constructor
    smVolumeMesh(smMeshType meshtype, smErrorLog *log);

    /// \brief destructor
    ~smVolumeMesh();

    /// \brief constructor
    void GenerateTetra(smChar *fileName);

    /// \brief load tetrahedron
    smBool LoadTetra(const smChar *fileName);

    /// \brief load surface triangles
    smBool getSurface(const smChar *fileName);

    /// \brief read the boundary conditions
    smBool readBC(const smChar *fileName);

    /// \brief initialize the surface mesh
    void initSurface();

    /// \brief copy the surface mesh
    void copySurface();

    /// \brief translate mesh vertices in x, y and z directions
    void translateVolumeMesh(smVec3<smFloat> p_translate);

    /// \brief scale the volume mesh
    void scaleVolumeMesh(smVec3<smFloat> p_scale);

    /// \brief rotate mesh vertices in x, y and z directions
    void rotVolumeMesh(smMatrix33<smFloat> p_rot);

    /// \brief load the mesh
    smBool loadMesh(smChar *fileName, smMeshFileType fileType);
};

#endif
