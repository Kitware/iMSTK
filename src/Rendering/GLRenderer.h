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
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/SceneObject.h"
#include "Core/Matrix.h"

class smMesh;
class AABB;
class smLineMesh;
class smViewer;
class Plane;
class Scene;

struct RenderDetail;
/// \brief gl rendering utilities
class smGLRenderer: public CoreClass
{
public:
    /// \brief constructor
    smGLRenderer();

    /// \brief draws surface mesh with given rendertdail and draw paramters
    static void drawSurfaceMeshTriangles(std::shared_ptr<smMesh> p_surfaceMesh, std::shared_ptr<RenderDetail> renderDetail);

    /// \brief  draw normals
    static void drawNormals(std::shared_ptr<smMesh> p_mesh, Color p_color = Color::colorBlue, float length=1.0);

    /// \brief  draw line mesh
    static void drawLineMesh(std::shared_ptr<smLineMesh> p_lineMesh, std::shared_ptr<RenderDetail> renderDetail);

    /// \brief draw coordinate system
    static void drawCoordSystem(std::shared_ptr<smViewer> viewer, std::string p_name, core::Vec3d p_pos, core::Vec3d dirX, core::Vec3d dirY, core::Vec3d dirZ);

    /// \brief begin rendering triangle
    static void beginTriangles();

    /// \brief draw triangle
    static void drawTriangle(core::Vec3d &p_1, core::Vec3d &p_2, core::Vec3d &p_3);

    /// \brief end triangle rendering
    static void endTriangles();

    /// \brief draw AABB
    static void draw(AABB &aabb, Color p_color = Color(1.0, 1.0, 1.0));

    /// \brief  draw plane
    static void draw(Plane &p_plane, float p_scale = 10.0, Color p_color = Color::colorYellow);

    /// \brief  draw axes given rotation matrix, position and length to draw
    static void drawAxes(const Matrix33f &rotMat, const core::Vec3f &pos, const float length);

    /// \brief  draw global axes length to draw
    static void drawAxes(const float length);

    /// \brief  draw arrow given starting position, ending poistion and width of the arrow head
    static void drawArrow(const core::Vec3f &start, const core::Vec3f &end, const float D);

    /// \brief draw function is called for visualization the object
    virtual void draw(){}

    /// \brief switch to default rendering
    static void enableDefaultGLRendering();

    /// \brief Renders an Scene's objects
    ///
    /// \detail Requires the caller to have called registerForScene() on p_scene
    /// \param p_scene The scene to render
    /// \param p_param The drawing parameters
    static void renderScene(std::shared_ptr<Scene> p_scene);
    /// \brief Renders a Scene's objects
    ///
    /// \detail Requires the caller to have called registerForScene() on p_scene
    /// \param p_scene The scene to render
    /// \param p_param The drawing parameters
    /// \param p_proj A custom projection matrix to use instead of that provided with the scene
    /// \param p_view A custom view matrix to use instead of that provided with the scene
    static void renderScene(std::shared_ptr<Scene> p_scene,
                            const Matrix44f &p_proj,
                            const Matrix44f &p_view);

    /// \brief Renders a single SceneObject
    ///
    /// \param p_sceneObject The scene object to render
    /// \param p_param The drawing parameters
    static void renderSceneObject(std::shared_ptr<SceneObject> p_sceneObject);
};

#endif
