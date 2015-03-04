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
#include "smUtilities/smDataStructs.h"
#include "smRendering/smViewer.h"
#include "smUtilities/smQuat.h"
#include "smRendering/smVAO.h"

smGLRenderer::smGLRenderer()
{

}

void smGLRenderer::drawLineMesh(smLineMesh *p_lineMesh, smRenderDetail *renderDetail)
{

    static smFloat shadowMatrixGL[16];
    static smVec3<smFloat> origin(0, 0, 0);
    static smVec3<smFloat> xAxis(1, 0, 0);
    static smVec3<smFloat> yAxis(0, 1, 0);
    static smVec3<smFloat> zAxis(0, 0, 1);

    if (renderDetail->renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    glDisable(GL_TEXTURE_2D);

    glPointSize(renderDetail->pointSize);
    glLineWidth(renderDetail->lineSize);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  renderDetail->colorDiffuse.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, renderDetail->colorSpecular.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, renderDetail->colorAmbient.toGLColor());
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, renderDetail->shininess);
    }

    glEnableClientState(GL_VERTEX_ARRAY);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_lineMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::activateTexture(p_lineMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    glVertexPointer(3, smGLRealType, 0, p_lineMesh->vertices.data());

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glTexCoordPointer(2, smGLRealType, 0, p_lineMesh->texCoord);
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_FACES)
    {
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, smGLUIntType, p_lineMesh->edges);
    }

    if ((renderDetail->renderType & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, smGLUIntType, p_lineMesh->edges);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_WIREFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(3.0, 2.0);
        glDisable(GL_LIGHTING);
        glDrawElements(GL_LINES, p_lineMesh->nbrEdges * 2, smGLUIntType, p_lineMesh->edges);
        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_LOCALAXIS)
    {
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_HIGHLIGHTVERTICES)
    {
        glDisable(GL_LIGHTING);
        glColor3fv((smGLReal*)&renderDetail->highLightColor);
        glDrawArrays(GL_POINTS, 0, p_lineMesh->nbrVertices);
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    glDisableClientState(GL_VERTEX_ARRAY);

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_lineMesh->isMeshTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_lineMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::disableTexture(p_lineMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP)
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

void smGLRenderer::drawSurfaceMeshTriangles(smMesh *p_surfaceMesh,
        smRenderDetail *renderDetail,
        smDrawParam p_drawParam)
{

    static smFloat shadowMatrixGL[16];
    static smVec3<smFloat> origin(0, 0, 0);
    static smVec3<smFloat> xAxis(1, 0, 0);
    static smVec3<smFloat> yAxis(0, 1, 0);
    static smVec3<smFloat> zAxis(0, 0, 1);
    smShader *shader = NULL;
    smBool shaderEnabled = false;
    smVAO *vao;

    if (renderDetail->renderType & SIMMEDTK_RENDER_VAO)
    {
        if (renderDetail->VAOs.size() < 1)
        {
            return;
        }
        else
        {
            if (renderDetail->VAOEnable[0])
            {
                vao = smVAO::getVAO(renderDetail->VAOs[0]);
            }
            else
            {
                return;
            }
        }
    }

    if (p_surfaceMesh->renderDetail.renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    if (p_drawParam.rendererObject->renderStage != SMRENDERSTAGE_SHADOWPASS)
    {
        for (smInt i = 0; i < renderDetail->shaders.size(); i++)
        {
            if (renderDetail->shaderEnable[i])
            {
                shader = smShader::getShader(renderDetail->shaders[i]);
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
    glPointSize(renderDetail->pointSize);
    glLineWidth(renderDetail->lineSize);

    if (p_surfaceMesh->vertTangents != NULL && shaderEnabled && p_surfaceMesh->tangentChannel && !(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glVertexAttribPointerARB(shader->tangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, p_surfaceMesh->vertTangents);
        glEnableVertexAttribArrayARB(shader->tangentAttrib);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_MATERIALCOLOR)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  renderDetail->colorDiffuse.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, renderDetail->colorSpecular.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, renderDetail->colorAmbient.toGLColor());
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, renderDetail->shininess);
    }

    if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            }

            if (!shaderEnabled)
            {
                for (smInt t = 0; t < p_surfaceMesh->textureIds.size(); t++)
                {
                    glActiveTexture(GL_TEXTURE0 + t);
                    smTextureManager::activateTexture(p_surfaceMesh->textureIds[t].textureId);
                }
            }
            else
            {
                shader->activeGLTextures(p_surfaceMesh->uniqueId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP && !(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glVertexPointer(3, smGLRealType, 0, p_surfaceMesh->vertices.data());
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE && !(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glTexCoordPointer(2, smGLRealType, 0, p_surfaceMesh->texCoord);
        }
    }

    if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glNormalPointer(smGLRealType, 0, p_surfaceMesh->vertNormals);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_FACES)
    {
        if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
        {
            glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        }
        else
        {
            vao->draw(p_drawParam);
        }
    }

    if ((renderDetail->renderType & (SIMMEDTK_RENDER_VERTICES)))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);

        if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
        {
            glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        }

        glEnable(GL_LIGHTING);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_WIREFRAME)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(renderDetail->lineSize + 0.5);
        glPolygonOffset(1.0, 1.0);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4fv(renderDetail->wireFrameColor.toGLColor());

        if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
        {
            glDrawElements(GL_TRIANGLES, p_surfaceMesh->nbrTriangles * 3, smGLUIntType, p_surfaceMesh->triangles);
        }

        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glLineWidth(renderDetail->lineSize);
        //default rendering
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_LOCALAXIS)
    {
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_HIGHLIGHTVERTICES && !(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glDisable(GL_LIGHTING);
        glColor3fv((smGLReal*)&renderDetail->highLightColor);
        glDrawArrays(GL_POINTS, 0, p_surfaceMesh->nbrVertices);
        glEnable(GL_LIGHTING);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TRANSPARENT)
    {
        glDisable(GL_BLEND);
    }

    if (!(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_TEXTURE)
    {
        if (p_surfaceMesh->isMeshTextured())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (smInt t = 0; t < p_surfaceMesh->textureIds.size(); t++)
            {
                glActiveTexture(GL_TEXTURE0 + t);
                smTextureManager::disableTexture(p_surfaceMesh->textureIds[t].textureId);
            }
        }
    }

    if (renderDetail->renderType & SIMMEDTK_RENDER_COLORMAP && !(renderDetail->renderType & SIMMEDTK_RENDER_VAO))
    {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    if (p_drawParam.rendererObject->renderStage != SMRENDERSTAGE_SHADOWPASS)
    {
        for (smInt i = 0; i < renderDetail->shaders.size(); i++)
        {
            if (shaderEnabled)
            {
                smShader::getShader(renderDetail->shaders[i])->posdraw(p_surfaceMesh);
                shader->disableShader();
                break;
            }
        }
    }

    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
}

void smGLRenderer::drawNormals(smMesh *p_mesh, smColor p_color)
{

    glDisable(GL_LIGHTING);
    glColor3fv((smGLFloat*)&p_color);
    smVec3f baryCenter;
    smVec3f tmp;

    glBegin(GL_LINES);

    for (smInt i = 0; i < p_mesh->nbrVertices; i++)
    {
        glVertex3fv((smGLFloat*) & (p_mesh->vertices[i]));
        tmp = p_mesh->vertices[i] + p_mesh->vertNormals[i] * 5;
        glVertex3fv((smGLFloat *)&tmp);
    }

    for (smInt i = 0; i < p_mesh->nbrTriangles; i++)
    {
        baryCenter = p_mesh->vertices[p_mesh->triangles[i].vert[0]] + p_mesh->vertices[p_mesh->triangles[i].vert[1]] + p_mesh->vertices[p_mesh->triangles[i].vert[2]] ;
        baryCenter = baryCenter / 3.0;
        glVertex3fv((smGLFloat*) & (baryCenter));
        tmp = baryCenter + p_mesh->triNormals[i] * 5;
        glVertex3fv((smGLFloat *)&tmp);
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

void smGLRenderer::beginTriangles()
{

    glBegin(GL_TRIANGLES);
}

void smGLRenderer::drawTriangle(smVec3<smFloat> &p_1, smVec3<smFloat> &p_2, smVec3<smFloat> &p_3)
{

    glVertex3fv((GLfloat*)&p_1);
    glVertex3fv((GLfloat*)&p_2);
    glVertex3fv((GLfloat*)&p_3);
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
    glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMin.z);
    glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMin.z);

    glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMin.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMin.z);

    glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMin.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMax.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMax.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMax.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMin.z);

    glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMax.z);

    glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMin.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMin.y, aabb.aabbMax.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMax.z);
    glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMin.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMin.z);
    glVertex3f(aabb.aabbMax.x, aabb.aabbMin.y, aabb.aabbMin.z);

    glVertex3f(aabb.aabbMax.x, aabb.aabbMax.y, aabb.aabbMin.z);
    glVertex3f(aabb.aabbMin.x, aabb.aabbMax.y, aabb.aabbMin.z);
    glEnd();
    glLineWidth(1.0);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}

void smGLRenderer::draw(smPlane &p_plane, smFloat p_scale, smColor p_color)
{

    smQuat<smFloat> rot;
    smFloat angle;
    smVec3f axisOfRot;
    smVec3f defaultDir(0, 0, 1);
    smVec3f planePoints[4] = {smVec3f(-p_scale, p_scale, 0),
                              smVec3f(-p_scale, -p_scale, 0),
                              smVec3f(p_scale, -p_scale, 0),
                              smVec3f(p_scale, p_scale, 0)
                             };
    smVec3f tmp;

    angle = acos(defaultDir.dot(p_plane.unitNormal));
    axisOfRot = p_plane.unitNormal.cross(defaultDir);
    axisOfRot.normalize();

    rot.fromAxisAngle(axisOfRot, -angle);

    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3fv(p_color.toGLColor());
    tmp = rot.rotate(planePoints[0]) + p_plane.pos;
    glVertex3fv((smGLFloat*)&tmp);
    tmp = rot.rotate(planePoints[1]) + p_plane.pos;
    glVertex3fv((smGLFloat*)&tmp);
    tmp = rot.rotate(planePoints[2]) + p_plane.pos;
    glVertex3fv((smGLFloat*)&tmp);
    tmp = rot.rotate(planePoints[3]) + p_plane.pos;
    glVertex3fv((smGLFloat*)&tmp);
    glEnd();
    glEnable(GL_LIGHTING);
}



void smGLRenderer::enableDefaultGLRendering()
{

    glDisable(GL_VERTEX_PROGRAM_ARB);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glUseProgramObjectARB(0);
}

void smGLRenderer::renderScene(smScene* p_scene,
                               smDrawParam p_param)
{
    smScene::smSceneIterator sceneIter;

    assert(p_scene);
    assert(p_param.projMatrix);
    assert(p_param.viewMatrix);

    //Load View and Projection Matrices
    // -- with new rendering techniques, these would be passed to a shader
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(p_param.projMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(p_param.viewMatrix);

    sceneIter.setScene(p_scene, p_param.caller);

    for (smInt j = sceneIter.start(); j < sceneIter.end(); j++)
    {
        renderSceneObject(sceneIter[j], p_param);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void smGLRenderer::renderSceneObject(smSceneObject* p_sceneObject,
                                     smDrawParam p_param)
{
    if (p_sceneObject->renderDetail.renderType & SIMMEDTK_RENDER_NONE)
    {
        return;
    }

    //if the custom rendering enable only render this
    if (p_sceneObject->renderDetail.renderType & SIMMEDTK_RENDER_CUSTOMRENDERONLY)
    {
        if (p_sceneObject->customRender != nullptr)
        {
            p_sceneObject->customRender->preDraw(p_sceneObject);
            p_sceneObject->customRender->draw(p_sceneObject);
            p_sceneObject->customRender->postDraw(p_sceneObject);
        }
    }
    else
    {
        //If there is custom renderer first render the preDraw function. which is responsible for
        //rendering before the default renderer takes place
        if (p_sceneObject->customRender != nullptr)
        {
            p_sceneObject->customRender->preDraw(p_sceneObject);
        }

        p_sceneObject->draw(p_param);

        //If there is custom renderer, render the postDraw function. which is responsible for
        //rendering after the default renderer takes place
        if (p_sceneObject->customRender != nullptr)
        {
            p_sceneObject->customRender->postDraw(p_sceneObject);
        }
    }
}
