/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#include "smSimulators/smFemSceneObject.h"
#include "omp.h"


//brief: build the LM matrix before the assembly of the stiffness matrix
//For info LM, ID structures refer 'The Finite element method : linear static and
//dynamic finite element analysis' by T.J.R Hughes
void smFemSceneObject::buildLMmatrix()
{
    smInt i, j;
    ID = MatrixXf::Zero(v_mesh->nbrNodes, 3);

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

    LM = MatrixXf::Zero(v_mesh->nbrTetra, 12);

    smInt dof = 0;

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
    fm_temp = VectorXf::Zero(totalDof);
    fm = VectorXf::Zero(totalDof);
    totalDisp = VectorXf::Zero(totalDof);
    displacements = VectorXf::Zero(totalDof);
    temp_displacements = VectorXf::Zero(totalDof);
    displacements_prev = VectorXf::Zero(totalDof);
    componentMasses = VectorXf::Zero(totalDof);
    Kinv = MatrixXf::Zero(totalDof, totalDof);
    stiffnessMatrix = MatrixXf::Zero(totalDof, totalDof);
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

        MatrixXf k_ele = MatrixXf::Zero(12, 12);
        MatrixXf Ndash = MatrixXf::Zero(4, 3);

        smFloat vol, dd;
        Matrix4f coff = Matrix4f::Zero();
        Matrix4f coffinv = Matrix4f::Zero();

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
            cout << "Inverting";
            Kinv = stiffnessMatrix.inverse();
            cout << "done" << endl;
            saveMatrix("models/Floppy_Kinv.dat", Kinv);
        }
    }

    //If the simulation is dynamic calculate the lumped mass matrix
    if (dynamicFem)
    {
        dymamic_temp = VectorXf::Zero(totalDof);
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
    smInt i, j;
    nodeMass = VectorXf::Zero(v_mesh->nbrNodes);
    Vector3f centroid;
    Vector3f node0, node1, node2, node3;
    Vector3f face012, face023, face013, face123;

    for (i = 0; i < v_mesh->nbrTetra; i++)
    {
        node0(0) = v_mesh->nodes[0].x;
        node0(1) = v_mesh->nodes[0].y;
        node0(2) = v_mesh->nodes[0].z;
        node1(0) = v_mesh->nodes[1].x;
        node1(1) = v_mesh->nodes[1].y;
        node1(2) = v_mesh->nodes[1].z;
        node2(0) = v_mesh->nodes[2].x;
        node2(1) = v_mesh->nodes[2].y;
        node2(2) = v_mesh->nodes[2].z;
        node3(0) = v_mesh->nodes[3].x;
        node3(1) = v_mesh->nodes[3].y;
        node3(2) = v_mesh->nodes[3].z;

        centroid(0) = 0.25 * (v_mesh->nodes[0].x + v_mesh->nodes[1].x + v_mesh->nodes[2].x + v_mesh->nodes[3].x);
        centroid(1) = 0.25 * (v_mesh->nodes[0].y + v_mesh->nodes[1].y + v_mesh->nodes[2].y + v_mesh->nodes[3].y);
        centroid(2) = 0.25 * (v_mesh->nodes[0].z + v_mesh->nodes[1].z + v_mesh->nodes[2].z + v_mesh->nodes[3].z);

        face012(0) = (1.0 / 3) * (v_mesh->nodes[0].x + v_mesh->nodes[1].x + v_mesh->nodes[2].x);
        face012(1) = (1.0 / 3) * (v_mesh->nodes[0].y + v_mesh->nodes[1].y + v_mesh->nodes[2].y);
        face012(2) = (1.0 / 3) * (v_mesh->nodes[0].z + v_mesh->nodes[1].z + v_mesh->nodes[2].z);

        face023(0) = (1.0 / 3) * (v_mesh->nodes[0].x + v_mesh->nodes[2].x + v_mesh->nodes[3].x);
        face023(1) = (1.0 / 3) * (v_mesh->nodes[0].y + v_mesh->nodes[2].y + v_mesh->nodes[3].y);
        face023(2) = (1.0 / 3) * (v_mesh->nodes[0].z + v_mesh->nodes[2].z + v_mesh->nodes[3].z);

        face013(0) = (1.0 / 3) * (v_mesh->nodes[0].x + v_mesh->nodes[1].x + v_mesh->nodes[3].x);
        face013(1) = (1.0 / 3) * (v_mesh->nodes[0].y + v_mesh->nodes[1].y + v_mesh->nodes[3].y);
        face013(2) = (1.0 / 3) * (v_mesh->nodes[0].z + v_mesh->nodes[1].z + v_mesh->nodes[3].z);

        face123(0) = (1.0 / 3) * (v_mesh->nodes[3].x + v_mesh->nodes[1].x + v_mesh->nodes[2].x);
        face123(1) = (1.0 / 3) * (v_mesh->nodes[3].y + v_mesh->nodes[1].y + v_mesh->nodes[2].y);
        face123(2) = (1.0 / 3) * (v_mesh->nodes[3].z + v_mesh->nodes[1].z + v_mesh->nodes[2].z);

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
smFloat smFemSceneObject::tetraVolume(Vector3f &a, Vector3f &b, Vector3f &c, Vector3f &d)
{
    Matrix4f det;
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
smBool smFemSceneObject::loadMatrix(const smChar *fname, MatrixXf &a)
{
    FILE *p;
    smInt i, j;
    smFloat temp;

    if (p = fopen(fname, "rb"))
    {
        smFloat* data = new smFloat[a.rows()*a.rows()];
        fread(data, sizeof(smFloat), a.rows()*a.rows() , p);

        for (i = 0; i < a.rows(); i++)
        {
            for (j = 0; j < a.rows(); j++)
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
smBool smFemSceneObject::saveMatrix(const smChar *fname, MatrixXf &a)
{
    FILE *p;
    smInt i, j;

    if (p = fopen(fname, "wb"))
    {
        smFloat *temp = new smFloat[a.rows()*a.rows()];

        //first copy the elements in a temporary buffer
        for (i = 0; i < a.rows(); i++)
        {
            for (j = 0; j < a.rows(); j++)
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
void smFemSceneObject::assembleK(smInt element, MatrixXf k)
{
    smInt i, j;

    for (i = 0; i < 12; i++)
    {
        if (LM(element, i) != 0)
        {
            for (j = 0; j < 12; j++)
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
    if (xyz == 1)
    {
        return v_mesh->nodes[v_mesh->tetra[tet].vert[xyz123 - 1] - 1].x;
    }

    if (xyz == 2)
    {
        return v_mesh->nodes[v_mesh->tetra[tet].vert[xyz123 - 1] - 1].y;
    }

    if (xyz == 3)
    {
        return v_mesh->nodes[v_mesh->tetra[tet].vert[xyz123 - 1] - 1].z;
    }
}

///brief: compute the displacement using reanalysis technique
void smFemSceneObject::calculateDisplacements_QStatic(smVec3<smFloat> *vertices)
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
            vertices[i].x = v_mesh->origVerts[i].x + displacements(ID(i, 0) - 1, 0);
            vertices[i].y = v_mesh->origVerts[i].y + displacements(ID(i, 1) - 1, 0);
            vertices[i].z = v_mesh->origVerts[i].z + displacements(ID(i, 2) - 1, 0);
        }
    }
}

///brief:: calculates the displacements dy explicit dynamics using Central difference scheme
void smFemSceneObject::calculateDisplacements_Dynamic(smVec3<smFloat> *vertices)
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
            vertices[i].x = v_mesh->origVerts[i].x + displacements(ID(i, 0) - 1, 0);
            vertices[i].y = v_mesh->origVerts[i].y + displacements(ID(i, 1) - 1, 0);
            vertices[i].z = v_mesh->origVerts[i].z + displacements(ID(i, 2) - 1, 0);
        }
    }
}


void smFemSceneObject::draw(smDrawParam p_params)
{
    p_params.caller = this;
    this->v_mesh->draw(p_params);
}
