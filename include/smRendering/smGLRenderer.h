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

// STL includes
#include <memory>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smUtilities/smMatrix.h"

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
    static void drawSurfaceMeshTriangles(std::shared_ptr<smMesh> p_surfaceMesh, std::shared_ptr<smRenderDetail> renderDetail);

    /// \brief  draw normals
    static void drawNormals(std::shared_ptr<smMesh> p_mesh, smColor p_color = smColor::colorBlue, smFloat length=1.0);

    /// \brief  draw line mesh
    static void drawLineMesh(std::shared_ptr<smLineMesh> p_lineMesh, std::shared_ptr<smRenderDetail> renderDetail);

    /// \brief draw coordinate system
    static void drawCoordSystem(std::shared_ptr<smViewer> viewer, smString p_name, smVec3d p_pos, smVec3d dirX, smVec3d dirY, smVec3d dirZ);

    /// \brief begin rendering triangle
    static void beginTriangles();

    /// \brief draw triangle
    static void drawTriangle(smVec3d &p_1, smVec3d &p_2, smVec3d &p_3);

    /// \brief end triangle rendering
    static void endTriangles();

    /// \brief draw AABB
    static void draw(smAABB &aabb, smColor p_color = smColor(1.0, 1.0, 1.0));

    /// \brief  draw plane
    static void draw(smPlane &p_plane, smFloat p_scale = 10.0, smColor p_color = smColor::colorYellow);

    static void drawAxes(GLdouble length=1.0);

    static void drawArrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2, GLdouble D);

    /// \brief draw function is called for visualization the object
    virtual void draw(){}

    /// \brief switch to default rendering
    static void enableDefaultGLRendering();

    /// \brief Renders an smScene's objects
    ///
    /// \detail Requires the caller to have called registerForScene() on p_scene
    /// \param p_scene The scene to render
    /// \param p_param The drawing parameters
    static void renderScene(std::shared_ptr<smScene> p_scene);
    /// \brief Renders a smScene's objects
    ///
    /// \detail Requires the caller to have called registerForScene() on p_scene
    /// \param p_scene The scene to render
    /// \param p_param The drawing parameters
    /// \param p_proj A custom projection matrix to use instead of that provided with the scene
    /// \param p_view A custom view matrix to use instead of that provided with the scene
    static void renderScene(std::shared_ptr<smScene> p_scene,
                            const smMatrix44f &p_proj,
                            const smMatrix44f &p_view);

    /// \brief Renders a single smSceneObject
    ///
    /// \param p_sceneObject The scene object to render
    /// \param p_param The drawing parameters
    static void renderSceneObject(std::shared_ptr<smSceneObject> p_sceneObject);
};

#endif
