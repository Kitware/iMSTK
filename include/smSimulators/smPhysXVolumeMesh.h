#ifndef SMPHYSXVOLUMEMESH_H
#define SMPHYSXVOLUMEMESH_H
#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smMesh/smVolumeMesh.h"
#include "smMesh/smSurfaceMesh.h"



struct smPhysXLink{
  smInt tetraIndex;
  smFloat baryCetricDistance[4];

};

struct smNeighborTetrasOfNode{
	smInt nbrNeiTet;
	smInt *idx;
};

class smPhysXVolumeMesh:public smVolumeMesh{
public:


  smBool renderSurface;
  smBool renderTetras;
  vector<smPhysXLink> links;
  smInt nbrLinks;
  smSurfaceMesh *surfaceMesh;
  
  vector<vector <smEdge>> tetraEdges;
  smNeighborTetrasOfNode *neiTet;
  
  smBool *drawTet;
  smBool *removedNode;

  

public:
  smPhysXVolumeMesh();
  ~smPhysXVolumeMesh(void);
  smBool loadTetFile(smChar *p_TetFileName, smChar *p_surfaceMesh);
  
  ///reflect the motions on surfacemesh
  void updateSurfaceVertices();
  virtual void draw(smDrawParam p_params);
  void createEdgeofTetras();
  //void rotVolumeMesh(smMatrix33<smFloat> p_rot);
 
  void findNeighborTetrasOfNode();	//brute force search
};



#endif