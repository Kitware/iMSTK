#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "smMesh/smVegaSceneObject.h"
#include "objMeshRender.h"
#include "objMeshEncode.h"

smVegaSceneObject::smVegaSceneObject(char * filename):
  mesh(NULL), meshRender(NULL), displayList(0), displayListExists(false), displayListEdges(0), displayListEdgesExists(false)
{
  int verbose = 0;
  mesh = std::make_shared<ObjMesh>(filename, verbose);

  int encStart = strlen(filename) - 4;
  if ((encStart > 0) && (strcmp(&filename[encStart], ".enc") == 0))
  {
    // must decode
    printf("Decoding mesh.\n");
    objMeshDecode(mesh.get());
    printf("Decoded mesh.\n");
  }

  meshRender = std::make_shared<ObjMeshRender>(mesh.get());
  if (meshRender->numTextures() > 0)
    hasTextures_ = true;
  else
    hasTextures_ = false;

  BuildFaceNormals();

  n = mesh->getNumVertices();
  renderMode = OBJMESHRENDER_SMOOTH | OBJMESHRENDER_MATERIAL;
}

smVegaSceneObject::~smVegaSceneObject()
{
  PurgeDisplayList();
}

void smVegaSceneObject::SetMaterialAlpha(double alpha)
{
  mesh->setMaterialAlpha(alpha);
}

void smVegaSceneObject::PurgeDisplayList()
{
  if (displayListExists)
    glDeleteLists(displayList, 1);
  displayListExists = false;

  if (displayListEdgesExists)
    glDeleteLists(displayListEdges, 1);
  displayListEdgesExists = false;
}

void smVegaSceneObject::BuildDisplayList()
{
  GLenum errorCode;
  const GLubyte * errorString;

  errorCode = glGetError();
  if (errorCode != GL_NO_ERROR)
  {
    errorString = gluErrorString(errorCode);
    printf("OpenGL Error (start of BuildDisplayList): %s\n", errorString);
  }

  if (displayListExists)
    glDeleteLists(displayList, 1);

  displayList = meshRender->createDisplayList(OBJMESHRENDER_TRIANGLES, renderMode);
  displayListExists = true;

  if (displayListEdgesExists)
    glDeleteLists(displayListEdges, 1);

  displayListEdges = meshRender->createDisplayList(OBJMESHRENDER_EDGES, renderMode);
  displayListEdgesExists = true;

  errorCode = glGetError();
  if (errorCode != GL_NO_ERROR)
  {
    errorString = gluErrorString(errorCode);
    printf("OpenGL Error (end of BuildDisplayList): %s\n", errorString);
  }
}

// assumes pre-existing face normals
// second parameter is treshold angle for hard edges
void smVegaSceneObject::BuildVertexNormals(double thresholdAngle)
{
  //do stuff with structure
  mesh->buildVertexNormals(thresholdAngle);
}

void smVegaSceneObject::BuildFaceNormals()
{
  mesh->buildFaceNormals();
}

void smVegaSceneObject::BuildNormals(double thresholdAngle)
{
  BuildFaceNormals();
  BuildVertexNormals(thresholdAngle);
}

void smVegaSceneObject::SetNormalsToFaceNormals()
{
  mesh->setNormalsToFaceNormals();
}

void smVegaSceneObject::BuildNormalsFancy(double thresholdAngle)
{
  BuildFaceNormals();
  mesh->buildVertexNormalsFancy(thresholdAngle);
}

void smVegaSceneObject::Render()
{
  if(displayListExists)
    glCallList(displayList);
  else
    meshRender->render(OBJMESHRENDER_TRIANGLES, renderMode);
}

void smVegaSceneObject::SetShadowingModelviewMatrix(double ground[4], double light[4])
{
  double dot;
  double shadowMat[4][4];

  dot = ground[0] * light[0] + ground[1] * light[1] + ground[2] * light[2] + ground[3] * light[3];

  shadowMat[0][0] = dot - light[0] * ground[0];
  shadowMat[1][0] = 0.0 - light[0] * ground[1];
  shadowMat[2][0] = 0.0 - light[0] * ground[2];
  shadowMat[3][0] = 0.0 - light[0] * ground[3];

  shadowMat[0][1] = 0.0 - light[1] * ground[0];
  shadowMat[1][1] = dot - light[1] * ground[1];
  shadowMat[2][1] = 0.0 - light[1] * ground[2];
  shadowMat[3][1] = 0.0 - light[1] * ground[3];

  shadowMat[0][2] = 0.0 - light[2] * ground[0];
  shadowMat[1][2] = 0.0 - light[2] * ground[1];
  shadowMat[2][2] = dot - light[2] * ground[2];
  shadowMat[3][2] = 0.0 - light[2] * ground[3];

  shadowMat[0][3] = 0.0 - light[3] * ground[0];
  shadowMat[1][3] = 0.0 - light[3] * ground[1];
  shadowMat[2][3] = 0.0 - light[3] * ground[2];
  shadowMat[3][3] = dot - light[3] * ground[3];

  glMultMatrixd((const GLdouble*)shadowMat);
}

void smVegaSceneObject::RenderShadow(double ground[4], double light[4])
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  SetShadowingModelviewMatrix(ground, light);

  bool texEnabled = AreTexturesEnabled();
  DisableTextures();

  if(displayListExists)
    glCallList(displayList);
  else
    meshRender->render(OBJMESHRENDER_TRIANGLES, renderMode);

  if (texEnabled)
    EnableTextures();

  glPopMatrix();
}

void smVegaSceneObject::RenderVertices()
{
  meshRender->render(OBJMESHRENDER_VERTICES, renderMode);
}

void smVegaSceneObject::RenderVertices_Selection()
{
  meshRender->render(OBJMESHRENDER_VERTICES, renderMode);
}

void smVegaSceneObject::RenderEdges()
{
  if(displayListEdgesExists)
    glCallList(displayListEdges);
  else
    meshRender->render(OBJMESHRENDER_EDGES, renderMode);
}

void smVegaSceneObject::RenderFacesAndEdges()
{
  meshRender->render(OBJMESHRENDER_TRIANGLES | OBJMESHRENDER_EDGES, renderMode);
}

void smVegaSceneObject::RenderEdgesInGroup(char * groupName)
{
  meshRender->renderGroupEdges(groupName);
}

void smVegaSceneObject::RenderVertices(int numVertices, int * vertexList)
{
  meshRender->renderSpecifiedVertices(vertexList, numVertices);
}

void smVegaSceneObject::RenderVertex(int vertex)
{
  meshRender->renderVertex(vertex);
}

int smVegaSceneObject::GetClosestVertex(Vec3d & queryPos, double * distance, double * auxVertexBuffer)
{
  return mesh->getClosestVertex(queryPos, distance);
}

// highlights vertex i, i=0,1,2,...,n-1
void smVegaSceneObject::HighlightVertex(int i)
{
  glColor3f(0,1,0);
  glPointSize(8.0);

  Vec3d pos = mesh->getPosition(i);

  glBegin(GL_POINTS);
    glVertex3f(pos[0], pos[1], pos[2]);
  glEnd();
}

bool smVegaSceneObject::AreTexturesEnabled()
{
  return ((renderMode && OBJMESHRENDER_TEXTURE) != 0);
}

void smVegaSceneObject::EnableTextures()
{
  renderMode = renderMode | OBJMESHRENDER_TEXTURE;
}

void smVegaSceneObject::DisableTextures()
{
  renderMode = renderMode & (~OBJMESHRENDER_TEXTURE);
}

int smVegaSceneObject::SetUpTextures(LightingModulationType lightingModulation, MipmapType mipmap, AnisotropicFilteringType anisotropicFiltering, TextureTransparencyType textureTransparency, std::vector<ObjMeshRender::Texture*> * texturePool, int updatePool)
{
  int textureMode = 0; // = OBJMESHRENDER_GL_MODULATE | OBJMESHRENDER_GL_NOMIPMAP | OBJMESHRENDER_GL_ANISOTROPICFILTERING

  switch(lightingModulation)
  {
    case REPLACE:
      textureMode |= OBJMESHRENDER_GL_REPLACE;
      break;
    case MODULATE:
      textureMode |= OBJMESHRENDER_GL_MODULATE;
      break;
  }

  switch(mipmap)
  {
    case USEMIPMAP:
      textureMode |= OBJMESHRENDER_GL_USEMIPMAP;
      break;
    case NOMIPMAP:
      textureMode |= OBJMESHRENDER_GL_NOMIPMAP;
      break;
  }

  switch(anisotropicFiltering)
  {
    case USEANISOTROPICFILTERING:
      textureMode |= OBJMESHRENDER_GL_USEANISOTROPICFILTERING;
      break;
    case NOANISOTROPICFILTERING:
      textureMode |= OBJMESHRENDER_GL_NOANISOTROPICFILTERING;
      break;
  }

  meshRender->loadTextures(textureMode, texturePool, updatePool);
  if (meshRender->numTextures() > 0)
    hasTextures_ = true;
  else
    hasTextures_ = false;

  EnableTextures();

  switch(textureTransparency)
  {
    case USETEXTURETRANSPARENCY:
      if (meshRender->maxBytesPerPixelInTextures() == 4)
        renderMode |= OBJMESHRENDER_TRANSPARENCY;
      break;
    case NOTEXTURETRANSPARENCY:
      break;
  }

  return 0;
}

void smVegaSceneObject::RenderNormals()
{
  double normalLength = 0.1;
  meshRender->renderNormals(normalLength);
}

void smVegaSceneObject::BuildNeighboringStructure()
{
  mesh->buildVertexFaceNeighbors();
}

void smVegaSceneObject::ComputeMeshGeometricParameters(Vec3d * centroid, double * radius)
{
  mesh->getMeshGeometricParameters(centroid, radius);
}

void smVegaSceneObject::ComputeMeshRadius(Vec3d & centroid, double * radius)
{
  mesh->getMeshRadius(centroid, radius);
}

void smVegaSceneObject::ExportMeshGeometry(int * numVertices, double ** vertices, int * numTriangles, int ** triangles)
{
  mesh->exportGeometry(numVertices, vertices, numTriangles, triangles, NULL, NULL);
}

void smVegaSceneObject::TransformRigidly(double * centerOfMass, double * R)
{
  Vec3d cv(centerOfMass);
  Mat3d Rv(R);
  mesh->transformRigidly(cv, Rv);
}

