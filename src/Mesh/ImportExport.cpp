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

#include "ImportExport.h"
#include "Mesh.h"
#include "Core/SDK.h"

/// \brief
smBool smImportExport::convertToJSON(smMesh *p_mesh, const smString& p_outputFileName,
                                     smExportOptions /*p_export*/)
{

    FILE *file;
    file = fopen(p_outputFileName.c_str(), "w");
    fprintf(file, "{ \n");

    if (file == NULL)
    {
        return false;
    }

    fprintf(file, "\t\"vertexPositions\" : \n[");

    for (int i = 0; i < p_mesh->nbrVertices - 1; i++)
    {
        fprintf(file, "%f,%f,%f,", p_mesh->vertices[i][0], p_mesh->vertices[i][1], p_mesh->vertices[i][2]);
    }

    fprintf(file, "%f,%f,%f", p_mesh->vertices[p_mesh->nbrVertices - 1][0], p_mesh->vertices[p_mesh->nbrVertices - 1][1], p_mesh->vertices[p_mesh->nbrVertices - 1][2]);
    fprintf(file, "],\n");

    fprintf(file, "\t\"vertexNormals\" : \n[");

    for (int i = 0; i < p_mesh->nbrVertices - 1; i++)
    {
        fprintf(file, "%f,%f,%f,", p_mesh->vertNormals[i][0], p_mesh->vertNormals[i][1], p_mesh->vertNormals[i][2]);
    }

    fprintf(file, "%f,%f,%f", p_mesh->vertNormals[p_mesh->nbrVertices - 1][0], p_mesh->vertNormals[p_mesh->nbrVertices - 1][1], p_mesh->vertNormals[p_mesh->nbrVertices - 1][2]);
    fprintf(file, "],\n");

    fprintf(file, "\t\"vertexTextureCoords\" : [");

    for (int i = 0; i < p_mesh->nbrVertices - 1; i++)
    {
        fprintf(file, "%f,%f,", p_mesh->texCoord[i].u, p_mesh->texCoord[i].v);
    }

    fprintf(file, "%f,%f", p_mesh->texCoord[p_mesh->nbrVertices - 1].u, p_mesh->texCoord[p_mesh->nbrVertices - 1].v);
    fprintf(file, "],\n");

    fprintf(file, "\t\"tangents\" : \n[");

    for (int i = 0; i < p_mesh->nbrVertices - 1; i++)
    {
        fprintf(file, "%f,%f,%f,", p_mesh->vertTangents[i][0], p_mesh->vertTangents[i][1], p_mesh->vertTangents[i][2]);
    }

    fprintf(file, "%f,%f,%f", p_mesh->vertTangents[p_mesh->nbrVertices - 1][0], p_mesh->vertTangents[p_mesh->nbrVertices - 1][1], p_mesh->vertTangents[p_mesh->nbrVertices - 1][2]);
    fprintf(file, "],\n");

    fprintf(file, "\t\"indices\" : \n[");

    for (int i = 0; i < p_mesh->nbrTriangles; i++)
    {
        fprintf(file, "%d,%d,%d,", p_mesh->triangles[i].vert[0], p_mesh->triangles[i].vert[1], p_mesh->triangles[i].vert[2]);
    }

    fprintf(file, "%d,%d,%d", p_mesh->triangles[p_mesh->nbrTriangles - 1].vert[0], p_mesh->triangles[p_mesh->nbrTriangles - 1].vert[1], p_mesh->triangles[p_mesh->nbrTriangles - 1].vert[2]);
    fprintf(file, "]\n");

    fprintf(file, "}");

    fclose(file);
    return true;
}
