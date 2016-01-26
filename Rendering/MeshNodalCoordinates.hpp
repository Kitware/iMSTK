// This file is part of the SimMedTK project.
// Copyright (c) Kitware, Inc.
//
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

#ifndef RENDERING_MESHNODALCOORDINATES_TXX
#define RENDERING_MESHNODALCOORDINATES_TXX

#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkVariant.h"
#include "vtkVariantCast.h"

namespace imstk {

//------------------------------------------------------------------------------
// Can't use vtkStandardNewMacro with a template.
template <class Scalar> MeshNodalCoordinates<Scalar> *
MeshNodalCoordinates<Scalar>::New()
{
    VTK_STANDARD_NEW_BODY(MeshNodalCoordinates<Scalar>)
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::PrintSelf(ostream &os, vtkIndent indent)
{
    this->MeshNodalCoordinates<Scalar>::Superclass::PrintSelf(
        os, indent);

    os << indent << "vertexArray : " << this->vertexArray << std::endl;
    os << "TempDoubleArray : " << this->TempDoubleArray << std::endl;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::Initialize()
{
    this->vertexArray = nullptr;
    this->MaxId = -1;
    this->Size = 0;
    this->NumberOfComponents = 1;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::GetTuples(vtkIdList *ptIds, vtkAbstractArray *output)
{
    vtkDataArray *outArray = vtkDataArray::FastDownCast(output);
    if(!outArray)
    {
        vtkWarningMacro( << "Input is not a vtkDataArray");
        return;
    }

    vtkIdType numTuples = ptIds->GetNumberOfIds();

    outArray->SetNumberOfComponents(this->NumberOfComponents);
    outArray->SetNumberOfTuples(numTuples);

    const vtkIdType numPoints = ptIds->GetNumberOfIds();
    for(vtkIdType i = 0; i < numPoints; ++i)
    {
        outArray->SetTuple(i, this->GetTuple(ptIds->GetId(i)));
    }
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *output)
{
    vtkDataArray *outArray = vtkDataArray::FastDownCast(output);
    if(!outArray)
    {
        vtkErrorMacro( << "Input is not a vtkDataArray");
        return;
    }

    if(outArray->GetNumberOfComponents() != this->GetNumberOfComponents())
    {
        vtkErrorMacro( << "Incorrect number of components in input array.");
        return;
    }

    for(vtkIdType outArrayTupleId = 0; p1 <= p2; ++p1)
    {
        outArray->SetTuple(outArrayTupleId++, this->GetTuple(p1));
    }
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::Squeeze()
{
    // noop
}

//------------------------------------------------------------------------------
template <class Scalar> vtkArrayIterator*
MeshNodalCoordinates<Scalar>::NewIterator()
{
    vtkErrorMacro( << "Not implemented.");
    return NULL;
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::LookupValue(vtkVariant value)
{
    bool valid = true;
    Scalar val = vtkVariantCast<Scalar>(value, &valid);
    if(valid)
    {
        return this->Lookup(val, 0);
    }
    return -1;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::LookupValue(vtkVariant value, vtkIdList *ids)
{
    bool valid = true;
    Scalar val = vtkVariantCast<Scalar>(value, &valid);
    ids->Reset();
    if(valid)
    {
        vtkIdType index = 0;
        while((index = this->Lookup(val, index)) >= 0)
        {
            ids->InsertNextId(index++);
        }
    }
}

//------------------------------------------------------------------------------
template <class Scalar> vtkVariant MeshNodalCoordinates<Scalar>
::GetVariantValue(vtkIdType idx)
{
    return vtkVariant(this->GetValueReference(idx));
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::ClearLookup()
{
    // no-op, no fast lookup implemented.
}

//------------------------------------------------------------------------------
template <class Scalar> double* MeshNodalCoordinates<Scalar>
::GetTuple(vtkIdType i)
{
    this->TempDoubleArray = (*this->vertexArray)[i];
    return this->TempDoubleArray.data();
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::GetTuple(vtkIdType i, double *tuple)
{
    tuple[0] = static_cast<double>((*this->vertexArray)[i][0]);
    tuple[1] = static_cast<double>((*this->vertexArray)[i][1]);
    tuple[2] = static_cast<double>((*this->vertexArray)[i][2]);
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::LookupTypedValue(Scalar value)
{
    return this->Lookup(value, 0);
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::LookupTypedValue(Scalar value, vtkIdList *ids)
{
    ids->Reset();
    vtkIdType index = 0;
    while((index = this->Lookup(value, index)) >= 0)
    {
        ids->InsertNextId(index++);
    }
}

//------------------------------------------------------------------------------
template <class Scalar> Scalar MeshNodalCoordinates<Scalar>
::GetValue(vtkIdType idx)
{
    return this->GetValueReference(idx);
}

//------------------------------------------------------------------------------
template <class Scalar> Scalar& MeshNodalCoordinates<Scalar>
::GetValueReference(vtkIdType idx)
{
    const vtkIdType tuple = idx / this->NumberOfComponents;
    const vtkIdType comp = idx % this->NumberOfComponents;
    switch(comp)
    {
        case 0:
            return (*this->vertexArray)[tuple](0);
        case 1:
            return (*this->vertexArray)[tuple](1);
        case 2:
            return (*this->vertexArray)[tuple](2);
        default:
            vtkErrorMacro( << "Invalid number of components.");
            static Scalar dummy(0);
            return dummy;
    }
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::GetTupleValue(vtkIdType tupleId, Scalar *tuple)
{
    tuple[0] = (*this->vertexArray)[tupleId](0);
    tuple[1] = (*this->vertexArray)[tupleId](1);
    tuple[2] = (*this->vertexArray)[tupleId](2);
}

//------------------------------------------------------------------------------
template <class Scalar> int MeshNodalCoordinates<Scalar>
::Allocate(vtkIdType, vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return 0;
}

//------------------------------------------------------------------------------
template <class Scalar> int MeshNodalCoordinates<Scalar>
::Resize(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return 0;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetNumberOfTuples(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetTuple(vtkIdType, vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetTuple(vtkIdType, const float *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetTuple(vtkIdType, const double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertTuple(vtkIdType, vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertTuple(vtkIdType, const float *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertTuple(vtkIdType, const double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertTuples(vtkIdList *, vtkIdList *, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertTuples(vtkIdType,vtkIdType,vtkIdType,vtkAbstractArray*)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::InsertNextTuple(vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::InsertNextTuple(const float *)
{

    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::InsertNextTuple(const double *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::DeepCopy(vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::DeepCopy(vtkDataArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InterpolateTuple(vtkIdType, vtkIdList *, vtkAbstractArray *, double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InterpolateTuple(vtkIdType, vtkIdType, vtkAbstractArray*, vtkIdType,
                   vtkAbstractArray*, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetVariantValue(vtkIdType, vtkVariant)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::RemoveTuple(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::RemoveFirstTuple()
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::RemoveLastTuple()
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetTupleValue(vtkIdType, const Scalar*)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertTupleValue(vtkIdType, const Scalar*)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::InsertNextTupleValue(const Scalar *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetValue(vtkIdType, Scalar)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::InsertNextValue(Scalar)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertValue(vtkIdType, Scalar)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::InsertVariantValue(vtkIdType, vtkVariant)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
template <class Scalar> MeshNodalCoordinates<Scalar>
::MeshNodalCoordinates()
{
}

//------------------------------------------------------------------------------
template <class Scalar> MeshNodalCoordinates<Scalar>
::~MeshNodalCoordinates()
{
}

//------------------------------------------------------------------------------
template <class Scalar> void MeshNodalCoordinates<Scalar>
::SetVertexArray(std::vector<Vector3<Scalar>> &vertices)
{
    Initialize();
    this->NumberOfComponents = 3;
    this->Size = this->NumberOfComponents * vertices.size();
    this->MaxId = this->Size - 1;
    this->vertexArray = &vertices;
    this->Modified();
}

//------------------------------------------------------------------------------
template <class Scalar> vtkIdType MeshNodalCoordinates<Scalar>
::Lookup(const Scalar &val, vtkIdType index)
{
    while(index <= this->MaxId)
    {
        if(this->GetValueReference(index++) == val)
        {
            return index;
        }
    }
    return -1;
}

}

#endif // MESHNODALCOORDINATES_H
