// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMGLRENDERER_H
#define SMGLRENDERER_H

#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"

class smMesh;
class smAABB;
class smLineMesh;
class smViewer;
class smPlane;

struct smRenderDetail;
/// \brief gl rendering utilities
class smGLRenderer: public smCoreClass
{
public:
    /// \brief constructor
    smGLRenderer();
    /// \brief draws surface mesh with given rendertdail and draw paramters
    static void drawSurfaceMeshTriangles(smMesh *p_surfaceMesh, smRenderDetail *renderDetail, smDrawParam p_drawParam);
    /// \brief  draw normals
    static void drawNormals(smMesh *p_mesh, smColor p_color = smColor::colorBlue);
    /// \brief  draw line mesh
    static void drawLineMesh(smLineMesh *p_lineMesh, smRenderDetail *renderDetail);
    /// \brief draw coordinate system
    static void drawCoordSystem(smViewer *viewer, smString p_name, smVec3<smFloat> p_pos, smVec3<smFloat> dirX, smVec3<smFloat> dirY, smVec3<smFloat> dirZ);
    /// \brief begin rendering triangle
    static void beginTriangles();
    /// \brief draw triangle
    static void drawTriangle(smVec3<smFloat> &p_1, smVec3<smFloat> &p_2, smVec3<smFloat> &p_3);
    /// \brief end triangle rendering
    static void endTriangles();
    /// \brief draw AABB
    static void draw(smAABB &aabb, smColor p_color = smColor(1.0, 1.0, 1.0));
    /// \brief  draw plane
    static void draw(smPlane &p_plane, smFloat p_scale = 10.0, smColor p_color = smColor::colorYellow);
    /// \brief switch to default rendering
    static void enableDefaultGLRendering();
    /// \brief Renders an smScene's objects
    ///
    /// \detail Requires the caller to have called registerForScene() on p_scene
    /// \param p_scene The scene to render
    /// \param p_param The drawing parameters
    static void renderScene(smScene* p_scene, smDrawParam p_param);
    /// \brief Renders a single smSceneObject
    ///
    /// \param p_sceneObject The scene object to render
    /// \param p_param The drawing parameters
    static void renderSceneObject(smSceneObject* p_sceneObject, smDrawParam p_param);
};

#endif
