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

#include "smSimulators/smFemSceneObject.h"

//brief: build the LM matrix before the assembly of the stiffness matrix
//For info LM, ID structures refer 'The Finite element method : linear static and
//dynamic finite element analysis' by T.J.R Hughes
void smFemSceneObject::buildLMmatrix()
{
    smInt i, j;
    ID = smMatrixf::Zero(v_mesh->nbrNodes, 3);

    smInt dofID = 0;

    for (i = 0; i < v_mesh->nbrNodes; i++)
    {
        if (v_mesh->fixed[i])
        {
            ID(i, 0) = ID(i, 1) = ID(i, 2) = 0;
        }
        else
        {
            dofID++;
            ID(i, 0) = dofID;
            dofID++;
            ID(i, 1) = dofID;
            dofID++;
            ID(i, 2) = dofID;
        }
    }

    totalDof = dofID;

    LM = smMatrixf::Zero(v_mesh->nbrTetra, 12);

    for (i = 0; i < v_mesh->nbrTetra; i++)
    {
        for (j = 0; j < 4; j++)
        {
            LM(i, 3 * j) = ID(v_mesh->tetra[i].vert[j] - 1, 0);
            LM(i, 3 * j + 1) = ID(v_mesh->tetra[i].vert[j] - 1, 1);
            LM(i, 3 * j + 2) = ID(v_mesh->tetra[i].vert[j] - 1, 2);
        }
    }

    //create arrays
    fm_temp = smVectorf::Zero(totalDof);
    fm = smVectorf::Zero(totalDof);
    totalDisp = smVectorf::Zero(totalDof);
    displacements = smVectorf::Zero(totalDof);
    temp_displacements = smVectorf::Zero(totalDof);
    displacements_prev = smVectorf::Zero(totalDof);
    componentMasses = smVectorf::Zero(totalDof);
    Kinv = smMatrixf::Zero(totalDof, totalDof);
    stiffnessMatrix = smMatrixf::Zero(totalDof, totalDof);
}

//brief: computes the stiffness matrix, its inverse and saves it in external file.
// In the next run, if that file is found, the inverse is just loaded without computation
void smFemSceneObject::computeStiffness()
{
    smInt i, j, a, b, k;
    FILE* check1;
    FILE* check2;

    if ((check1 = fopen("models/Floppy_K.dat", "rb")) &&
            (check2 = fopen("models/Floppy_Kinv.dat", "rb")))
    {
        loadMatrix("models/Floppy_K.dat", stiffnessMatrix);

        if (!dynamicFem)
        {
            loadMatrix("models/Floppy_Kinv.dat", Kinv);
        }
    }
    else
    {
        smFloat e, v;
        e = 1e5; // Pascal
        v = 0.45; // approaches .5 for more realistic results

        smFloat c1 = e * (1 - v) / ((1 + v) * (1 - 2 * v));
        smFloat c2 = v / (1 - v);
        smFloat c3 = (1 - 2 * v) / (2 * (1 - v));

        smMatrixf k_ele = smMatrixf::Zero(12, 12);
        smMatrixf Ndash = smMatrixf::Zero(4, 3);

        smFloat vol;
        smMatrix44f coff = smMatrix44f::Zero();
        smMatrix44f coffinv = smMatrix44f::Zero();

        for (i = 0; i < v_mesh->nbrTetra; i++)
        {
            vol = (1 / 6.0) * abs((V(1, 2, i) - V(1, 1, i)) * ((V(2, 3, i) - V(2, 1, i)) * (V(3, 4, i) - V(3, 1, i)) - (V(2, 4, i) - V(2, 1, i)) * (V(3, 3, i) - V(3, 1, i)))
                                  - (V(2, 2, i) - V(2, 1, i)) * ((V(1, 3, i) - V(1, 1, i)) * (V(3, 4, i) - V(3, 1, i)) - (V(1, 4, i) - V(1, 1, i)) * (V(3, 3, i) - V(3, 1, i)))
                                  + (V(3, 2, i) - V(3, 1, i)) * ((V(1, 3, i) - V(1, 1, i)) * (V(3, 4, i) - V(3, 1, i)) - (V(1, 4, i) - V(1, 1, i)) * (V(3, 3, i) - V(3, 1, i))));

            for (k = 0; k < 3; k++)
            {
                for (j = 0; j < 4; j++)
                {
                    coff(k, j) = V(k + 1, j + 1, i);
                }
            }

            coff(3, 0) = 1.0;
            coff(3, 1) = 1.0;
            coff(3, 2) = 1.0;
            coff(3, 3) = 1.0;
            coffinv = coff.inverse();

            for (a = 0; a < 4; a++)
            {
                for (b = 0; b < 4; b++)
                {
                    k_ele(3 * a, 3 * b) = (coffinv(a, 0) * c1 * coffinv(b, 0) + coffinv(a, 2) * c3 * c1 * coffinv(b, 2) + coffinv(a, 1) * c1 * c3 * coffinv(b, 1)) * vol;
                    k_ele(3 * a + 1, 3 * b) = (coffinv(a, 1) * c1 * c2 * coffinv(b, 0) + coffinv(a, 0) * c1 * c3 * coffinv(b, 1)) * vol;
                    k_ele(3 * a + 2, 3 * b) = (coffinv(a, 2) * c1 * c2 * coffinv(b, 0) + coffinv(a, 0) * c1 * c3 * coffinv(b, 2)) * vol;

                    k_ele(3 * a, 3 * b + 1) = (coffinv(a, 0) * c1 * c2 * coffinv(b, 1) + coffinv(a, 1) * c1 * c3 * coffinv(b, 0)) * vol;
                    k_ele(3 * a + 1, 3 * b + 1) = (coffinv(a, 1) * c1 * coffinv(b, 1) + coffinv(a, 2) * c1 * c3 * coffinv(b, 2) + coffinv(a, 0) * c1 * c3 * coffinv(b, 0)) * vol;
                    k_ele(3 * a + 2, 3 * b + 1) = (coffinv(a, 2) * c1 * c2 * coffinv(b, 1) + coffinv(a, 1) * c1 * c3 * coffinv(b, 2)) * vol;

                    k_ele(3 * a, 3 * b + 2) = (coffinv(a, 0) * c1 * c2 * coffinv(b, 2) + coffinv(a, 2) * c1 * c3 * coffinv(b, 0)) * vol;
                    k_ele(3 * a + 1, 3 * b + 2) = (coffinv(a, 1) * c1 * c2 * coffinv(b, 2) + coffinv(a, 2) * c1 * c3 * coffinv(b, 1)) * vol;
                    k_ele(3 * a + 2, 3 * b + 2) = (coffinv(a, 2) * c1 * coffinv(b, 2) + coffinv(a, 1) * c1 * c3 * coffinv(b, 1) + coffinv(a, 0) * c1 * c3 * coffinv(b, 0)) * vol;
                }
            }

            assembleK(i, k_ele);
        }

        saveMatrix("models/Floppy_K.dat", stiffnessMatrix);

        if (!dynamicFem)
        {
            std::cout << "Inverting";
            Kinv = stiffnessMatrix.inverse();
            std::cout << "done" << "\n";
            saveMatrix("models/Floppy_Kinv.dat", Kinv);
        }
    }

    //If the simulation is dynamic calculate the lumped mass matrix
    if (dynamicFem)
    {
        dymamic_temp = smVectorf::Zero(totalDof);
        lumpMasses();

        for (i = 0; i < v_mesh->nbrNodes; i++)
        {
            if (ID(i, 0) != 0)
            {
                componentMasses(ID(i, 0) - 1) = nodeMass(i);
                componentMasses(ID(i, 1) - 1) = nodeMass(i);
                componentMasses(ID(i, 2) - 1) = nodeMass(i);
            }
        }

        componentMasses = componentMasses * density;
        viscosity = 0.8 * componentMasses;
    }
}

///brief: Mass lumping of the stiffness matrix
void smFemSceneObject::lumpMasses()
{
    smInt i;
    nodeMass = smVectorf::Zero(v_mesh->nbrNodes);
    smVec3f centroid;
    smVec3f node0, node1, node2, node3;
    smVec3f face012, face023, face013, face123;

    for (i = 0; i < v_mesh->nbrTetra; i++)
    {
        node0(0) = v_mesh->nodes[0][0];
        node0(1) = v_mesh->nodes[0][1];
        node0(2) = v_mesh->nodes[0][2];
        node1(0) = v_mesh->nodes[1][0];
        node1(1) = v_mesh->nodes[1][1];
        node1(2) = v_mesh->nodes[1][2];
        node2(0) = v_mesh->nodes[2][0];
        node2(1) = v_mesh->nodes[2][1];
        node2(2) = v_mesh->nodes[2][2];
        node3(0) = v_mesh->nodes[3][0];
        node3(1) = v_mesh->nodes[3][1];
        node3(2) = v_mesh->nodes[3][2];

        centroid(0) = 0.25 * (v_mesh->nodes[0][0] + v_mesh->nodes[1][0] + v_mesh->nodes[2][0] + v_mesh->nodes[3][0]);
        centroid(1) = 0.25 * (v_mesh->nodes[0][1] + v_mesh->nodes[1][1] + v_mesh->nodes[2][1] + v_mesh->nodes[3][1]);
        centroid(2) = 0.25 * (v_mesh->nodes[0][2] + v_mesh->nodes[1][2] + v_mesh->nodes[2][2] + v_mesh->nodes[3][2]);

        face012(0) = (1.0 / 3) * (v_mesh->nodes[0][0] + v_mesh->nodes[1][0] + v_mesh->nodes[2][0]);
        face012(1) = (1.0 / 3) * (v_mesh->nodes[0][1] + v_mesh->nodes[1][1] + v_mesh->nodes[2][1]);
        face012(2) = (1.0 / 3) * (v_mesh->nodes[0][2] + v_mesh->nodes[1][2] + v_mesh->nodes[2][2]);

        face023(0) = (1.0 / 3) * (v_mesh->nodes[0][0] + v_mesh->nodes[2][0] + v_mesh->nodes[3][0]);
        face023(1) = (1.0 / 3) * (v_mesh->nodes[0][1] + v_mesh->nodes[2][1] + v_mesh->nodes[3][1]);
        face023(2) = (1.0 / 3) * (v_mesh->nodes[0][2] + v_mesh->nodes[2][2] + v_mesh->nodes[3][2]);

        face013(0) = (1.0 / 3) * (v_mesh->nodes[0][0] + v_mesh->nodes[1][0] + v_mesh->nodes[3][0]);
        face013(1) = (1.0 / 3) * (v_mesh->nodes[0][1] + v_mesh->nodes[1][1] + v_mesh->nodes[3][1]);
        face013(2) = (1.0 / 3) * (v_mesh->nodes[0][2] + v_mesh->nodes[1][2] + v_mesh->nodes[3][2]);

        face123(0) = (1.0 / 3) * (v_mesh->nodes[3][0] + v_mesh->nodes[1][0] + v_mesh->nodes[2][0]);
        face123(1) = (1.0 / 3) * (v_mesh->nodes[3][1] + v_mesh->nodes[1][1] + v_mesh->nodes[2][1]);
        face123(2) = (1.0 / 3) * (v_mesh->nodes[3][2] + v_mesh->nodes[1][2] + v_mesh->nodes[2][2]);

        nodeMass(v_mesh->tetra[i].vert[0] - 1) = nodeMass(v_mesh->tetra[i].vert[0] - 1) + tetraVolume(node0, centroid, face012, face013);
        nodeMass(v_mesh->tetra[i].vert[0] - 1) = nodeMass(v_mesh->tetra[i].vert[0] - 1) + tetraVolume(node0, centroid, face023, face013);

        nodeMass(v_mesh->tetra[i].vert[1] - 1) = nodeMass(v_mesh->tetra[i].vert[1] - 1) + tetraVolume(node1, centroid, face123, face013);
        nodeMass(v_mesh->tetra[i].vert[1] - 1) = nodeMass(v_mesh->tetra[i].vert[1] - 1) + tetraVolume(node1, centroid, face012, face013);

        nodeMass(v_mesh->tetra[i].vert[2] - 1) = nodeMass(v_mesh->tetra[i].vert[2] - 1) + tetraVolume(node2, centroid, face012, face013);
        nodeMass(v_mesh->tetra[i].vert[2] - 1) = nodeMass(v_mesh->tetra[i].vert[2] - 1) + tetraVolume(node2, centroid, face023, face013);

        nodeMass(v_mesh->tetra[i].vert[3] - 1) = nodeMass(v_mesh->tetra[i].vert[3] - 1) + tetraVolume(node3, centroid, face023, face123);
        nodeMass(v_mesh->tetra[i].vert[3] - 1) = nodeMass(v_mesh->tetra[i].vert[3] - 1) + tetraVolume(node3, centroid, face012, face123);
    }
}

///brief: volume of a tetra
smFloat smFemSceneObject::tetraVolume(smVec3f &a, smVec3f &b, smVec3f &c, smVec3f &d)
{
    smMatrix44f det;
    smFloat vol;

    det(0, 0) = a(0);
    det(0, 1) = a(1);
    det(0, 2) = a(2);

    det(1, 0) = b(0);
    det(1, 1) = b(1);
    det(1, 2) = b(2);

    det(2, 0) = c(0);
    det(2, 1) = c(1);
    det(2, 2) = c(2);

    det(3, 0) = d(0);
    det(3, 1) = d(1);
    det(3, 2) = d(2);

    det(0, 3) = 1.0;
    det(1, 3) = 1.0;
    det(2, 3) = 1.0;
    det(3, 3) = 1.0;

    vol = (1.0 / 6.0) * abs(det.determinant());

    return vol;
}

///brief:reads matrix elements from an external file
smBool smFemSceneObject::loadMatrix(const smString &fname, smMatrixf &a)
{
    FILE *p = fopen(fname.c_str(), "rb");

    if (p)
    {
        smFloat* data = new smFloat[a.rows()*a.rows()];
        fread(data, sizeof(smFloat), a.rows()*a.rows() , p);

        for (smInt i = 0; i < a.rows(); i++)
        {
            for (smInt j = 0; j < a.rows(); j++)
            {
                a(i, j) = data[i * a.rows() + j];
            }
        }

        fclose(p);
        delete []data;
        return true;
    }

    return false;
}

///brief:saves the matrix to a external file
smBool smFemSceneObject::saveMatrix(const smString &fname, smMatrixf &a)
{
    FILE *p = fopen(fname.c_str(), "wb");

    if (p)
    {
        smFloat *temp = new smFloat[a.rows()*a.rows()];

        //first copy the elements in a temporary buffer
        for (smInt i = 0; i < a.rows(); i++)
        {
            for (smInt j = 0; j < a.rows(); j++)
            {
                temp[i + j * a.rows()] = a(i, j);
            }
        }

        //write
        fwrite(temp, sizeof(smFloat), a.rows()*a.rows(), p);
        fclose(p);
        delete []temp;
        return true;
    }

    return false;
}


///brief:assembles element stiffness to global stiffness
void smFemSceneObject::assembleK(smInt element, smMatrixf k)
{
    for (smInt i = 0; i < 12; i++)
    {
        if (LM(element, i) != 0)
        {
            for (smInt j = 0; j < 12; j++)
            {
                if (LM(element, j) != 0)
                {
                    stiffnessMatrix(LM(element, i) - 1, LM(element, j) - 1) =
                        stiffnessMatrix(LM(element, i) - 1, LM(element, j) - 1) + k(i, j);
                }
            }
        }
    }
}

///brief:
smFloat smFemSceneObject::V(smInt xyz, smInt xyz123, smInt tet)
{
    return v_mesh->nodes[v_mesh->tetra[tet].vert[xyz123 - 1] - 1][xyz-1];
}

///brief: compute the displacement using reanalysis technique
void smFemSceneObject::calculateDisplacements_QStatic(smVec3f *vertices)
{
    smInt i;
    smInt dofNumber = ID(pulledNode, 0);
    totalDisp.setZero(totalDof);

    if (pulledNode >= 0)
    {
        k33 = Kinv.block(dofNumber - 1, dofNumber - 1, 3, 3);
        k33inv = k33.inverse();

        fm_temp(dofNumber - 1, 0) = k33inv(0, 0) * displacements(dofNumber - 1, 0)
                                    + k33inv(0, 1) * displacements(dofNumber, 0)
                                    + k33inv(0, 2) * displacements(dofNumber + 1, 0);

        fm_temp(dofNumber, 0) = k33inv(1, 0) * displacements(dofNumber - 1, 0)
                                + k33inv(1, 1) * displacements(dofNumber, 0)
                                + k33inv(1, 2) * displacements(dofNumber + 1, 0);

        fm_temp(dofNumber + 1, 0) = k33inv(2, 0) * displacements(dofNumber - 1, 0)
                                    + k33inv(2, 1) * displacements(dofNumber, 0)
                                    + k33inv(2, 2) * displacements(dofNumber + 1, 0);

        smFloat total = 0;

        for (i = 0; i < totalDof; i++)
        {
            total = Kinv(i, dofNumber - 1) * fm_temp(dofNumber - 1, 0)
                    + Kinv(i, dofNumber) * fm_temp(dofNumber, 0)
                    + Kinv(i, dofNumber + 1) * fm_temp(dofNumber + 1, 0);

            totalDisp(i, 0) = totalDisp(i, 0) + total;
        }
    }

    fm = fm_temp;

    //Now, update the displacements array
    for (i = 0; i < totalDof; i++)
    {
        displacements(i, 0) = totalDisp(i, 0);
    }

    //update the vertices
    for (int i = 0; i < v_mesh->nbrVertices; i++)
    {
        if (!v_mesh->fixed[i])
        {
            vertices[i][0] = v_mesh->origVerts[i][0] + displacements(ID(i, 0) - 1, 0);
            vertices[i][1] = v_mesh->origVerts[i][1] + displacements(ID(i, 1) - 1, 0);
            vertices[i][2] = v_mesh->origVerts[i][2] + displacements(ID(i, 2) - 1, 0);
        }
    }
}

///brief:: calculates the displacements dy explicit dynamics using Central difference scheme
void smFemSceneObject::calculateDisplacements_Dynamic(smVec3f *vertices)
{
    smInt i;
    pulledNode = 62; //to test//324
    smInt dofNumber = ID(pulledNode - 1, 0);
    //to test
    displacements(dofNumber - 1, 0) = 0.0;
    displacements(dofNumber, 0) += 0.02;
    displacements(dofNumber + 1, 0) = 0.0;

    dymamic_temp = dT * dT * stiffnessMatrix * displacements;
    //store a copy before modifying
    temp_displacements = displacements;

    for (i = 0; i < (dofNumber - 1); i++)
    {
        displacements(i) = (1 / (componentMasses(i) + (dT / 2) * viscosity(i))) * (-dymamic_temp(i) + 2 * componentMasses(i) * displacements(i)
                           - (componentMasses(i) - (dT / 2) * viscosity(i)) * displacements_prev(i));
    }

    for (i = dofNumber + 2; i < totalDof; i++)
    {
        displacements(i) = (1 / (componentMasses(i) + (dT / 2) * viscosity(i))) * (-dymamic_temp(i) + 2 * componentMasses(i) * displacements(i)
                           - (componentMasses(i) - (dT / 2) * viscosity(i)) * displacements_prev(i));
    }

    //Now, update the displacements array
    displacements_prev = temp_displacements;

    //update the vertices
    for (i = 0; i < v_mesh->nbrVertices; i++)
    {
        if (!v_mesh->fixed[i])
        {
            vertices[i][0] = v_mesh->origVerts[i][0] + displacements(ID(i, 0) - 1, 0);
            vertices[i][1] = v_mesh->origVerts[i][1] + displacements(ID(i, 1) - 1, 0);
            vertices[i][2] = v_mesh->origVerts[i][2] + displacements(ID(i, 2) - 1, 0);
        }
    }
}


void smFemSceneObject::draw(const smDrawParam &p_params)
{
//     p_params.caller = this;
    this->v_mesh->draw(p_params);
}
smFemSceneObject::smFemSceneObject( smErrorLog *p_log )
{
    type = SIMMEDTK_SMFEMSCENEOBJECT;
    v_mesh = new smVolumeMesh( SMMESH_DEFORMABLE, p_log );
    pullUp = true;
    dynamicFem = false;

    if ( dynamicFem )
    {
        dT = 0.02;
        density = 500;
    }
}

smSceneObject *smFemSceneObject::clone()
{
    return this;
}

void smFemSceneObject::serialize( void */*p_memoryBlock*/ )
{
}

void smFemSceneObject::unSerialize( void */*p_memoryBlock*/ )
{

}

// void smFemSceneObject::init() {}
