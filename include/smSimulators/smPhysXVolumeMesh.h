#ifndef SMPHYSXVOLUMEMESH_H
#define SMPHYSXVOLUMEMESH_H
#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smMesh/smVolumeMesh.h"
#include "smMesh/smSurfaceMesh.h"

/// \brief !!
struct smPhysXLink{
	smInt tetraIndex;
	smFloat baryCetricDistance[4];
};

/// \brief neighbor tetrahedra of the node
struct smNeighborTetrasOfNode{
	smInt nbrNeiTet;
	smInt *idx;
};

/// \brief PhysX volume mesh
class smPhysXVolumeMesh:public smVolumeMesh{
public:
	smBool renderSurface; ///> true if surface is to be rendered 
	smBool renderTetras; ///> true if tetrahedra is to be rendered
	vector<smPhysXLink> links; ///> !!
	smInt nbrLinks; ///> !! number of links
	smSurfaceMesh *surfaceMesh; ///> surface mesh
	vector<vector <smEdge>> tetraEdges; ///> edges of tetrahedra
	smNeighborTetrasOfNode *neiTet; ///> neighbor tetrahedra of a node
	smBool *drawTet; ///> true for those tetra to be rendered
	smBool *removedNode; ///> true for nodes to be removed

	/// \brief constructor
	smPhysXVolumeMesh();

	/// \brief destructor
	~smPhysXVolumeMesh(void);

	/// \brief load the tetrahedra file in the PhysX format
	smBool loadTetFile(smChar *p_TetFileName, smChar *p_surfaceMesh);

	/// \brief !! update the surface vertices
	void updateSurfaceVertices();

	/// \brief draw PhysX volume mesh
	virtual void draw(smDrawParam p_params);

	/// \brief compute the edges of the tetrahedra
	void createEdgeofTetras();

	/// \brief find neighbor tetrahedra of node (brute force search)
	void findNeighborTetrasOfNode(); 
};

#endif
