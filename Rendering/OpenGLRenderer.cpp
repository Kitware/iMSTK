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

#include "Core/DataStructures.h"
#include "Core/Quaternion.h"
#include "Core/RenderDelegate.h"
#include "Core/Geometry.h"
#include "Mesh/SurfaceMesh.h"
#include "Rendering/OpenGLRenderer.h"
#include "Rendering/OpenGLViewer.h"
#include "Rendering/TextureManager.h"
#include "Rendering/VAO.h"

#ifndef _MSC_VER
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
// VTK glew library
#include <vtk_glew.h>

#ifndef __APPLE__
#  include <GL/gl.h> // for GLfloat, etc.
#  include <GL/glu.h> // for GLfloat, etc.
#else
#  include <OpenGL/gl.h> // for GLfloat, etc.
#endif
OpenGLRenderer::OpenGLRenderer()
{

}

#if 0
void OpenGLRenderer::drawLineMesh(std::shared_ptr<LineMesh> p_lineMesh, std::shared_ptr<RenderDetail> renderDetail)
{
    static core::Vec3d origin(0, 0, 0);
    static core::Vec3d xAxis(1, 0, 0);
    static core::Vec3d yAxis(0, 1, 0);
    static core::Vec3d zAxis(0, 0, 1);

    const int renderType = renderDetail->getRenderType();

    if (renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glDisable(GL_TEXTURE_2D);

    glPointSize(renderDetail->getPointSize());
    glLineWidth(renderDetail->getLineSize());

    if (renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  renderDetail->getColorDiffuse().toGLColor<GLfloat>());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, renderDetail->getColorSpecular().toGLColor<GLfloat>());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, renderDetail->getColorAmbient().toGLColor<GLfloat>());
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, renderDetail->getShininess());
    }

    glEnableClientState(GL_VERTEX_ARRAY);

    if (renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            for (size_t t = 0; t < p_lineMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                TextureManager::activateTexture(p_lineMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    glVertexPointer(3, GL_FLOAT, 0, p_lineMesh->vertices.data());

    if (renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glTexCoordPointer(2, GL_FLOAT, 0, p_lineMesh->texCoord);
        }
    }

    if (renderType & SIMMEDTK_RENDER_FACES)
    {
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, GL_UNSIGNED_INT, p_lineMesh->edges);
    }

    if ((renderType & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, GL_UNSIGNED_INT, p_lineMesh->edges);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderType & SIMMEDTK_RENDER_WIREFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(3.0, 2.0);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, GL_UNSIGNED_INT, p_lineMesh->edges);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderType & SIMMEDTK_RENDER_LOCALAXIS)
    {
        glEnable(GL_LIGHTING);
    }

    if (renderType & SIMMEDTK_RENDER_HIGHLIGHTVERTICES)
    {
        glDisable(GL_LIGHTING);
        glColor3fv(renderDetail->getHighLightColor().toGLColor<GLfloat>());
        glDrawArrays(GL_POINTS, 0, p_lineMesh->nbrVertices);
        glEnable(GL_LIGHTING);
    }

    if (renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    glDisableClientState(GL_VERTEX_ARRAY);

    if (renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (size_t t = 0; t < p_lineMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                TextureManager::disableTexture(p_lineMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}
#endif // 0

void OpenGLRenderer::drawSurfaceMeshTriangles(
    std::shared_ptr<SurfaceMesh> p_surfaceMesh,
    std::shared_ptr<RenderDetail> renderDetail)
{
    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glPointSize(renderDetail->getPointSize());
    glLineWidth(renderDetail->getLineSize());

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        const GLfloat * color = renderDetail->getColorDiffuse().toGLColor<GLfloat>();
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, renderDetail->getColorSpecular().toGLColor<GLfloat>());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, renderDetail->getColorAmbient().toGLColor<GLfloat>());
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, renderDetail->getShininess());
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, p_surfaceMesh->getVertices().data()->data());
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_DOUBLE, 0, p_surfaceMesh->getVertexNormals().data()->data());

    auto &meshTextures = p_surfaceMesh->getTextures();
    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->getRenderDelegate()->isTargetTextured())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, p_surfaceMesh->getTextureCoordinates().data()->data());

            for (size_t t = 0, end = meshTextures.size(); t < end; ++t)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                TextureManager::activateTexture(meshTextures[t]->textureName);
            }
        }
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_COLORMAP)
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_FACES)
    {
        // TODO: Investigate why this expensive copy needs to be performed.
        // Sean: I can only think because the data MUST be unsigned int or
        //     GLuint would really be the most appropriate, as per
        //     GL_UNSIGNED_INT.  Also the vector could be a std::array.
        std::vector<unsigned int> data;
        for(auto t : p_surfaceMesh->getTriangles())
        {
            data.emplace_back((unsigned int) t[0]);
            data.emplace_back((unsigned int) t[1]);
            data.emplace_back((unsigned int) t[2]);
        }
        glDrawElements(GL_TRIANGLES, data.size(), GL_UNSIGNED_INT, data.data());
    }

    if ((p_surfaceMesh->getRenderDetail()->getRenderType() & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        glColor3fv(renderDetail->getVertexColor().toGLColor<GLfloat>());
        glDrawElements(GL_TRIANGLES, p_surfaceMesh->getTriangles().size() * 3, GL_UNSIGNED_INT, p_surfaceMesh->getTriangles().data()->data());

        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_WIREFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(renderDetail->getLineSize() + 0.5);
        glPolygonOffset(1.0, 1.0);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4fv(renderDetail->getWireFrameColor().toGLColor<GLfloat>());

        glDrawElements(GL_TRIANGLES, p_surfaceMesh->getTriangles().size() * 3, GL_UNSIGNED_INT, p_surfaceMesh->getTriangles().data()->data());

        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glLineWidth(renderDetail->getLineSize());
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_HIGHLIGHTVERTICES)
    {
        glDisable(GL_LIGHTING);
        glColor3fv(renderDetail->getHighLightColor().toGLColor<GLfloat>());
        glDrawArrays(GL_POINTS, 0, p_surfaceMesh->getNumberOfVertices());
        glEnable(GL_LIGHTING);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->getRenderDelegate()->isTargetTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (size_t t = 0, end = meshTextures.size(); t < end; ++t)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                TextureManager::disableTexture(p_surfaceMesh->getTextureId(t));
            }
        }
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_COLORMAP)
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

void OpenGLRenderer::drawNormals(std::shared_ptr<SurfaceMesh> p_mesh, Color p_color, float length)
{
    glDisable(GL_LIGHTING);
    glColor3fv(reinterpret_cast<GLfloat*>(&p_color));
    core::Vec3d baryCenter;
    core::Vec3d tmp;

    glBegin(GL_LINES);

    auto &vertices = p_mesh->getVertices();
    for (size_t i = 0, end = vertices.size(); i < end; i++)
    {
        glVertex3dv(vertices[i].data());
        tmp = vertices[i] + p_mesh->getVertexNormal(i) * length;
        glVertex3dv(tmp.data());
    }

    auto const &triangles = p_mesh->getTriangles();
    for (size_t i = 0, end = p_mesh->getTriangles().size(); i < end; i++)
    {
        baryCenter = vertices[triangles[i][0]] + vertices[triangles[i][1]] + vertices[triangles[i][2]];
        baryCenter = baryCenter / 3.0;
        glVertex3dv(baryCenter.data());
        tmp = baryCenter + p_mesh->getTriangleNormal(i) * length;
        glVertex3dv(tmp.data());
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

void OpenGLRenderer::beginTriangles()
{
    glBegin(GL_TRIANGLES);
}

void OpenGLRenderer::drawTriangle(core::Vec3d &p_1, core::Vec3d &p_2, core::Vec3d &p_3)
{
    glVertex3dv(p_1.data());
    glVertex3dv(p_2.data());
    glVertex3dv(p_3.data());
}

void OpenGLRenderer::endTriangles()
{
    glEnd();
}

void OpenGLRenderer::draw(const Eigen::AlignedBox3d &aabb, Color p_color)
{
    glPushAttrib(GL_LIGHTING_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(1.0);
    glColor3fv(p_color.toGLColor<GLfloat>());
    glBegin(GL_LINES);

    auto const &min = aabb.min();
    auto const &max = aabb.max();

    glVertex3d(min(0), min(0), min(2));
    glVertex3d(max(0), min(0), min(2));

    glVertex3d(min(0), min(0), min(2));
    glVertex3d(min(0), max(0), min(2));

    glVertex3d(min(0), min(0), min(2));
    glVertex3d(min(0), min(0), max(2));

    glVertex3d(max(0), max(0), max(2));
    glVertex3d(min(0), max(0), max(2));

    glVertex3d(max(0), max(0), max(2));
    glVertex3d(max(0), min(0), max(2));

    glVertex3d(max(0), max(0), max(2));
    glVertex3d(max(0), max(0), min(2));

    glVertex3d(min(0), max(0), max(2));
    glVertex3d(min(0), min(0), max(2));

    glVertex3d(min(0), max(0), max(2));
    glVertex3d(min(0), max(0), min(2));

    glVertex3d(max(0), min(0), max(2));
    glVertex3d(min(0), min(0), max(2));

    glVertex3d(max(0), min(0), max(2));
    glVertex3d(max(0), min(0), min(2));

    glVertex3d(max(0), max(0), min(2));
    glVertex3d(max(0), min(0), min(2));

    glVertex3d(max(0), max(0), min(2));
    glVertex3d(min(0), max(0), min(2));
    glEnd();
    glLineWidth(1.0);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}

void OpenGLRenderer::drawArrow(const core::Vec3f &start, const core::Vec3f &end, const float D)
{
    float x = end[0] - start[0];
    float y = end[1] - start[1];
    float z = end[2] - start[2];
    float L = sqrt(x*x + y*y + z*z);

    GLUquadricObj *quadObj;

    glPushMatrix();

    glTranslated(start[0], start[1], start[2]);

    if ((x != 0.) || (y != 0.)) {
        glRotated(atan2(y, x) / 0.0174533, 0., 0., 1.);
        glRotated(atan2(sqrt(x*x + y*y), z) / 0.0174533, 0., 1., 0.);
    }
    else if (z<0){
        glRotated(180, 1., 0., 0.);
    }

    glTranslatef(0, 0, L - 4 * D);

    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 2 * D, 0.0, 4 * D, 32, 1);
    gluDeleteQuadric(quadObj);

    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluDisk(quadObj, 0.0, 2 * D, 32, 1);
    gluDeleteQuadric(quadObj);

    glTranslatef(0, 0, -L + 4 * D);

    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, D, D, L - 4 * D, 32, 1);
    gluDeleteQuadric(quadObj);

    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluDisk(quadObj, 0.0, D, 32, 1);
    gluDeleteQuadric(quadObj);

    glPopMatrix();

}

void OpenGLRenderer::drawAxes(const float length)
{
    glDisable(GL_LIGHTING);

    float headWidth = length / 12;

    Eigen::Vector3f origin(0, 0, 0);

    glColor3fv(Color::colorRed.toGLColor<GLfloat>());
    glPushMatrix();
    drawArrow(origin, Eigen::Vector3f(length, 0, 0), headWidth);
    glPopMatrix();

    glColor3fv(Color::colorGreen.toGLColor<GLfloat>());
    glPushMatrix();
    drawArrow(origin, Eigen::Vector3f(0, length, 0), headWidth);
    glPopMatrix();

    glColor3fv(Color::colorBlue.toGLColor<GLfloat>());
    glPushMatrix();
    drawArrow(origin, Eigen::Vector3f(0, 0, length), headWidth);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void OpenGLRenderer::drawAxes(const Matrix33f &rotMat, const core::Vec3f &pos, const float length)
{
    glDisable(GL_LIGHTING);

    GLfloat headWidth = length / 12;

    glColor3fv(Color::colorRed.toGLColor<GLfloat>());
    glPushMatrix();
    Eigen::Vector3f xVec(length, 0, 0);
    xVec = rotMat*xVec + pos;
    drawArrow(pos, xVec, headWidth);
    glPopMatrix();

    glColor3fv(Color::colorGreen.toGLColor<GLfloat>());
    glPushMatrix();
    Eigen::Vector3f yVec(0, length, 0);
    yVec = rotMat*yVec + pos;
    drawArrow(pos, yVec, headWidth);
    glPopMatrix();

    glColor3fv(Color::colorBlue.toGLColor<GLfloat>());
    glPushMatrix();
    Eigen::Vector3f zVec(0, 0, length);
    zVec = rotMat*zVec + pos;
    drawArrow(pos, zVec, headWidth);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void OpenGLRenderer::draw(Plane &p_plane, float p_scale, Color p_color)
{

    double angle;
    core::Vec3d axisOfRot;
    core::Vec3d defaultDir(0, 0, 1);
    core::Vec3d planePoints[4] = {core::Vec3d(-p_scale, p_scale, 0),
                              core::Vec3d(-p_scale, -p_scale, 0),
                              core::Vec3d(p_scale, -p_scale, 0),
                              core::Vec3d(p_scale, p_scale, 0)
                             };
    core::Vec3d tmp;

    core::Vec3d normal = p_plane.getUnitNormal();
    core::Vec3d point = p_plane.getPoint();
    angle = std::acos(defaultDir.dot(normal));
    axisOfRot = normal.cross(defaultDir);
    axisOfRot.normalize();

    Quaterniond rot = getRotationQuaternion(-angle,axisOfRot);

    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3fv(p_color.toGLColor<GLfloat>());
    tmp = rot*planePoints[0] + point;
    glVertex3dv(tmp.data());
    tmp = rot*planePoints[1] + point;
    glVertex3dv(tmp.data());
    tmp = rot*planePoints[2] + point;
    glVertex3dv(tmp.data());
    tmp = rot*planePoints[3] + point;
    glVertex3dv(tmp.data());
    glEnd();
    glEnable(GL_LIGHTING);
}

void OpenGLRenderer::renderScene(std::shared_ptr<Scene> p_scene)
{
    assert(p_scene);

    Matrix44f proj = p_scene->getCamera()->getProjMat();
    Matrix44f view = p_scene->getCamera()->getViewMat();

    renderScene(p_scene, proj, view);
}

void OpenGLRenderer::renderScene(std::shared_ptr<Scene> p_scene,
                               const Matrix44f &p_proj,
                               const Matrix44f &p_view)
{
    SceneLocal sceneLocal;

    assert(p_scene);

    //Load View and Projection Matrices
    // -- with new rendering techniques, these would be passed to a shader
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(p_proj.data());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(p_view.data());

    p_scene->copySceneToLocal(sceneLocal);

    //Enable lights
    p_scene->activateLights();
    p_scene->placeLights();

    for (auto x: sceneLocal.sceneObjects)
    {
        renderSceneObject(x);
    }

    p_scene->deactivateLights();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void OpenGLRenderer::renderSceneObject(std::shared_ptr<SceneObject> p_sceneObject)
{
    RenderDetail::Ptr detail = p_sceneObject->getRenderDetail();
    if (!detail || detail->getRenderType() & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    //if the custom rendering enable only render this
    RenderDelegate::Ptr delegate = p_sceneObject->getRenderDelegate();
    std::shared_ptr<CustomRenderer> renderer = p_sceneObject->getRenderer();
    if (detail->getRenderType() & SIMMEDTK_RENDER_CUSTOMRENDERONLY)
    {
        if (renderer != nullptr)
        {
            renderer->preDrawObject(*p_sceneObject);
            renderer->drawObject(*p_sceneObject);
            renderer->postDrawObject(*p_sceneObject);
        }
    }
    else if (delegate)
    {
        //If there is custom renderer first render the preDraw function. which is responsible for
        //rendering before the default renderer takes place
        if (renderer != nullptr)
        {
            renderer->preDrawObject(*p_sceneObject);
        }

        delegate->draw();

        //If there is custom renderer, render the postDraw function. which is responsible for
        //rendering after the default renderer takes place
        if (renderer != nullptr)
        {
            renderer->postDrawObject(*p_sceneObject);
        }
    }
}
