/*
  A library to load and render an obj mesh (called here a "scene object").
  This library depends on the "objMesh" library. See also objMesh.h.
*/

#ifndef _SIMMEDTK_VEGASCENEOBJECT_H_
#define _SIMMEDTK_VEGASCENEOBJECT_H_

#ifdef WIN32
  #include <windows.h>
#endif

//Simmedtk includes
#include "Core/Config.h"
#include "Core/ErrorLog.h"
#include "Core/CoreClass.h"

#include "vec3d.h"
#include "objMesh.h"

class VegaSceneObject
{
public:
  // create a static scene object, by loading it from an Alias Wavefront OBJ file
  VegaSceneObject(char * filename);
  virtual ~VegaSceneObject();

  // ==== mesh info and geometric queries ====

  inline int Getn() { return n; }
  inline int GetNumVertices() { return n; }
  inline int GetNumFaces() { return mesh->getNumFaces(); }
  inline std::shared_ptr<ObjMesh> GetMesh() { return mesh; }

  // smallest ball radius that encloses the model, with the ball centered at the given centroid
  void ComputeMeshRadius(Vec3d & centroid, double * radius);
  // compute mesh centroid and smallest enclosing radius
  void ComputeMeshGeometricParameters(Vec3d * centroid, double * radius);
  // export mesh data
  void ExportMeshGeometry(int * numVertices, double ** vertices, int * numTriangles, int ** triangles);

  // finds the closest vertex using an exhaustive search
  // returns distance in "distance", if distance is not NULL
  // in this class, you can safely ignore the last parameter (keep it NULL)
  virtual int GetClosestVertex(Vec3d & queryPos, double * distance=NULL, double * auxVertexBuffer=NULL);

  // ==== normals ====

  void BuildFaceNormals();
  void BuildNeighboringStructure();  // must be called before the vertex-normal functions below

  // second parameter is treshold angle for hard edges:
  void BuildVertexNormals(double thresholdAngle=85.0); // assumes pre-existing face normals
  void BuildNormals(double thresholdAngle=85.0); // builds both face and vertex normals
  void BuildNormalsFancy(double thresholdAngle=85.0);  // rebuilds facet normals + calls vertex-per-triangle normal update

  void SetNormalsToFaceNormals();

  // ==== transformation ====
  virtual void TransformRigidly(double * centerOfMass, double * R);

protected:
  int n;
  std::shared_ptr<ObjMesh> mesh;
};

#endif
