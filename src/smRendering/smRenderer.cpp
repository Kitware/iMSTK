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

#include "smRendering/smGLRenderer.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smDataStructures.h"
#include "smRendering/smViewer.h"
#include "smUtilities/smQuaternion.h"
#include "smRendering/smVAO.h"

smGLRenderer::smGLRenderer()
{

}

void smGLRenderer::drawLineMesh(std::shared_ptr<smLineMesh> p_lineMesh, std::shared_ptr<smRenderDetail> renderDetail)
{
    static smVec3d origin(0, 0, 0);
    static smVec3d xAxis(1, 0, 0);
    static smVec3d yAxis(0, 1, 0);
    static smVec3d zAxis(0, 0, 1);

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
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  renderDetail->getColorDiffuse().toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, renderDetail->getColorSpecular().toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, renderDetail->getColorAmbient().toGLColor());
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
                smTextureManager::activateTexture(p_lineMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    glVertexPointer(3, smGLRealType, 0, p_lineMesh->vertices.data());

    if (renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glTexCoordPointer(2, smGLRealType, 0, p_lineMesh->texCoord);
        }
    }

    if (renderType & SIMMEDTK_RENDER_FACES)
    {
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, smGLUIntType, p_lineMesh->edges);
    }

    if ((renderType & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, smGLUIntType, p_lineMesh->edges);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderType & SIMMEDTK_RENDER_WIREFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(3.0, 2.0);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, smGLUIntType, p_lineMesh->edges);
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
        glColor3fv(renderDetail->getHighLightColor().toGLColor());
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
                smTextureManager::disableTexture(p_lineMesh->textureIds[t].textureId);
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

void smGLRenderer::drawSurfaceMeshTriangles(std::shared_ptr<smMesh> p_surfaceMesh,
        std::shared_ptr<smRenderDetail> renderDetail,
        const smDrawParam &p_drawParam)
{
    static smVec3d origin(0, 0, 0);
    static smVec3d xAxis(1, 0, 0);
    static smVec3d yAxis(0, 1, 0);
    static smVec3d zAxis(0, 0, 1);
    std::shared_ptr<smShader> shader = nullptr;
    smBool shaderEnabled = false;
    std::shared_ptr<smVAO> vao;

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO)
    {
        if (renderDetail->getVAOs().size() < 1)
        {
            return;
        }
        else
        {
            if (renderDetail->getVAOEnable()[0])
            {
                vao = smVAO::getVAO(renderDetail->getVAOs()[0]);
            }
            else
            {
                return;
            }
        }
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    if (p_drawParam.rendererObject->renderStage != SMRENDERSTAGE_SHADOWPASS)
    {
        for (size_t i = 0; i < renderDetail->getShaders().size(); i++)
        {
            if (renderDetail->getShaderEnable()[i])
            {
                shader = smShader::getShader(renderDetail->getShaders()[i]);
                shader->enableShader();
                shader->predraw(p_surfaceMesh);
                shaderEnabled = true;
                break;
            }
        }
    }
    else
    {
        smGLRenderer::enableDefaultGLRendering();
    }

    glDisable(GL_TEXTURE_2D);
    glPointSize(renderDetail->getPointSize());
    glLineWidth(renderDetail->getLineSize());

    if (p_surfaceMesh->vertTangents != NULL && shaderEnabled && p_surfaceMesh->tangentChannel && !(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glVertexAttribPointerARB(shader->getTangentAttributes(), 3, GL_FLOAT, GL_FALSE, 0, p_surfaceMesh->vertTangents);
        glEnableVertexAttribArrayARB(shader->getTangentAttributes());
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        const GLfloat * color = renderDetail->getColorDiffuse().toGLColor();
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, renderDetail->getColorSpecular().toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, renderDetail->getColorAmbient().toGLColor());
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, renderDetail->getShininess());
    }

    if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            }

            if (!shaderEnabled)
            {
                for (size_t t = 0; t < p_surfaceMesh->textureIds.size(); t++)
                {
                    glActiveTexture(GL_TEXTURE0 + t);
                    smTextureManager::activateTexture(p_surfaceMesh->textureIds[t].textureId);
                }
            }
            else
            {
                shader->activeGLTextures(p_surfaceMesh->getUniqueId());
            }
        }
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_COLORMAP && !(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glVertexPointer(3, GL_DOUBLE, 0, p_surfaceMesh->vertices.data());
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TEXTURE && !(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glTexCoordPointer(2, smGLRealType, 0, p_surfaceMesh->texCoord);
        }
    }

    if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glNormalPointer(GL_DOUBLE, 0, p_surfaceMesh->vertNormals);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_FACES)
    {
        if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
        {
            glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        }
        else
        {
            vao->draw(p_drawParam);
        }
    }

    if ((p_surfaceMesh->getRenderDetail()->getRenderType() & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);

        if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
        {
            glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        }

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
        glColor4fv(renderDetail->getWireFrameColor().toGLColor());

        if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
        {
            glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        }

        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glLineWidth(renderDetail->getLineSize());
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_LOCALAXIS)
    {
        glEnable(GL_LIGHTING);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_HIGHLIGHTVERTICES && !(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glDisable(GL_LIGHTING);
        glColor3fv(renderDetail->getHighLightColor().toGLColor());
        glDrawArrays(GL_POINTS, 0, p_surfaceMesh->nbrVertices);
        glEnable(GL_LIGHTING);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    if (!(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (size_t t = 0; t < p_surfaceMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::disableTexture(p_surfaceMesh->textureIds[t].textureId);
            }
        }
    }

    if (p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_COLORMAP && !(p_surfaceMesh->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_VAO))
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    if (p_drawParam.rendererObject->renderStage != SMRENDERSTAGE_SHADOWPASS)
    {
        for (size_t i = 0; i < renderDetail->getShaders().size(); i++)
        {
            if (shaderEnabled)
            {
                smShader::getShader(renderDetail->getShaders()[i])->posdraw(p_surfaceMesh);
                shader->disableShader();
                break;
            }
        }
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

void smGLRenderer::drawNormals(std::shared_ptr<smMesh> p_mesh, smColor p_color)
{

    glDisable(GL_LIGHTING);
    glColor3fv(reinterpret_cast<smGLReal*>(&p_color));
    smVec3d baryCenter;
    smVec3d tmp;

    glBegin(GL_LINES);

    for (smInt i = 0; i < p_mesh->nbrVertices; i++)
    {
        glVertex3dv(p_mesh->vertices[i].data());
        tmp = p_mesh->vertices[i] + p_mesh->vertNormals[i] * 5;
        glVertex3dv(tmp.data());
    }

    for (smInt i = 0; i < p_mesh->nbrTriangles; i++)
    {
        baryCenter = p_mesh->vertices[p_mesh->triangles[i].vert[0]] + p_mesh->vertices[p_mesh->triangles[i].vert[1]] + p_mesh->vertices[p_mesh->triangles[i].vert[2]] ;
        baryCenter = baryCenter / 3.0;
        glVertex3dv(baryCenter.data());
        tmp = baryCenter + p_mesh->triNormals[i] * 5;
        glVertex3dv(tmp.data());
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

void smGLRenderer::beginTriangles()
{

    glBegin(GL_TRIANGLES);
}

void smGLRenderer::drawTriangle(smVec3d &p_1, smVec3d &p_2, smVec3d &p_3)
{

    glVertex3dv(p_1.data());
    glVertex3dv(p_2.data());
    glVertex3dv(p_3.data());
}

void smGLRenderer::endTriangles()
{

    glEnd();
}

void smGLRenderer::draw(smAABB &aabb, smColor p_color)
{

    glPushAttrib(GL_LIGHTING_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(1.0);
    glColor3fv(p_color.toGLColor());
    glBegin(GL_LINES);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMin[0], aabb.aabbMin[2]);
    glVertex3d(aabb.aabbMax[0], aabb.aabbMin[0], aabb.aabbMin[2]);

    glVertex3d(aabb.aabbMin[0], aabb.aabbMin[0], aabb.aabbMin[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMax[0], aabb.aabbMin[2]);

    glVertex3d(aabb.aabbMin[0], aabb.aabbMin[0], aabb.aabbMin[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMin[0], aabb.aabbMax[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMax[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMax[0], aabb.aabbMax[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMax[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMax[0], aabb.aabbMin[0], aabb.aabbMax[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMax[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMax[0], aabb.aabbMax[0], aabb.aabbMin[2]);

    glVertex3d(aabb.aabbMin[0], aabb.aabbMax[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMin[0], aabb.aabbMax[2]);

    glVertex3d(aabb.aabbMin[0], aabb.aabbMax[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMax[0], aabb.aabbMin[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMin[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMin[0], aabb.aabbMax[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMin[0], aabb.aabbMax[2]);
    glVertex3d(aabb.aabbMax[0], aabb.aabbMin[0], aabb.aabbMin[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMax[0], aabb.aabbMin[2]);
    glVertex3d(aabb.aabbMax[0], aabb.aabbMin[0], aabb.aabbMin[2]);

    glVertex3d(aabb.aabbMax[0], aabb.aabbMax[0], aabb.aabbMin[2]);
    glVertex3d(aabb.aabbMin[0], aabb.aabbMax[0], aabb.aabbMin[2]);
    glEnd();
    glLineWidth(1.0);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}

void smGLRenderer::draw(smPlane &p_plane, smFloat p_scale, smColor p_color)
{

    double angle;
    smVec3d axisOfRot;
    smVec3d defaultDir(0, 0, 1);
    smVec3d planePoints[4] = {smVec3d(-p_scale, p_scale, 0),
                              smVec3d(-p_scale, -p_scale, 0),
                              smVec3d(p_scale, -p_scale, 0),
                              smVec3d(p_scale, p_scale, 0)
                             };
    smVec3d tmp;

    angle = std::acos(defaultDir.dot(p_plane.unitNormal));
    axisOfRot = p_plane.unitNormal.cross(defaultDir);
    axisOfRot.normalized();

    smQuaterniond rot = getRotationQuaternion(-angle,axisOfRot);

    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3fv(p_color.toGLColor());
    tmp = rot*planePoints[0] + p_plane.pos;
    glVertex3dv(tmp.data());
    tmp = rot*planePoints[1] + p_plane.pos;
    glVertex3dv(tmp.data());
    tmp = rot*planePoints[2] + p_plane.pos;
    glVertex3dv(tmp.data());
    tmp = rot*planePoints[3] + p_plane.pos;
    glVertex3dv(tmp.data());
    glEnd();
    glEnable(GL_LIGHTING);
}

void smGLRenderer::enableDefaultGLRendering()
{

    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glUseProgramObjectARB(0);
}

void smGLRenderer::renderScene(std::shared_ptr<smScene> p_scene,
                               smDrawParam p_param)
{
    assert(p_scene);

    smMatrix44f proj = Eigen::Map<smMatrix44f>(p_scene->getCamera()->getProjMatRef());
    smMatrix44f view = Eigen::Map<smMatrix44f>(p_scene->getCamera()->getViewMatRef());

    renderScene(p_scene, p_param, proj, view);
}

void smGLRenderer::renderScene(std::shared_ptr<smScene> p_scene,
                               smDrawParam p_param,
                               const smMatrix44f &p_proj,
                               const smMatrix44f &p_view)
{
    smSceneIterator sceneIter;

    assert(p_scene);

    //Load View and Projection Matrices
    // -- with new rendering techniques, these would be passed to a shader
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(p_proj.data());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(p_view.data());

    sceneIter.setScene(p_scene, p_param.caller);

    //Enable lights
    p_scene->enableLights();
    p_scene->placeLights();

    for (smInt j = sceneIter.start(); j < sceneIter.end(); j++)
    {
        renderSceneObject(sceneIter[j], p_param);
    }

    p_scene->disableLights();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void smGLRenderer::renderSceneObject(std::shared_ptr<smSceneObject> p_sceneObject,
                                     const smDrawParam &p_param)
{
    if (p_sceneObject->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    //if the custom rendering enable only render this
    std::shared_ptr<smCustomRenderer> renderer = p_sceneObject->getRenderer();
    if (p_sceneObject->getRenderDetail()->getRenderType() & SIMMEDTK_RENDER_CUSTOMRENDERONLY)
    {
        if (renderer != nullptr)
        {
            renderer->preDraw(*p_sceneObject);
            renderer->draw(*p_sceneObject);
            renderer->postDraw(*p_sceneObject);
        }
    }
    else
    {
        //If there is custom renderer first render the preDraw function. which is responsible for
        //rendering before the default renderer takes place
        if (renderer != nullptr)
        {
            renderer->preDraw(*p_sceneObject);
        }

        // TODO: scenobject does not have a draw function
        p_sceneObject->draw(p_param);

        //If there is custom renderer, render the postDraw function. which is responsible for
        //rendering after the default renderer takes place
        if (renderer != nullptr)
        {
            renderer->postDraw(*p_sceneObject);
        }
    }
}
