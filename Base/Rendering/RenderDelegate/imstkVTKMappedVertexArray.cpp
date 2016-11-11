/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkVTKMappedVertexArray.h"

#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkVariant.h"
#include "vtkVariantCast.h"

namespace imstk
{

vtkStandardNewMacro(VTKMappedVertexArray);

void
VTKMappedVertexArray::PrintSelf(ostream &os, vtkIndent indent)
{
    this->VTKMappedVertexArray::Superclass::PrintSelf(
        os, indent);

    os << indent << "vertexArray : " << this->vertexArray << std::endl;
    os << "TempDoubleArray : " << this->TempDoubleArray << std::endl;
}

void
VTKMappedVertexArray::SetVertexArray(std::vector<Vec3d>& vertices)
{
    this->Initialize();
    this->NumberOfComponents = 3;
    this->Size = this->NumberOfComponents * vertices.size();
    this->MaxId = this->Size - 1;
    this->vertexArray = &vertices;
    this->Modified();
}

void
VTKMappedVertexArray::Initialize()
{
    this->vertexArray = nullptr;
    this->MaxId = -1;
    this->Size = 0;
    this->NumberOfComponents = 1;
}

void
VTKMappedVertexArray::GetTuples(vtkIdList *ptIds, vtkAbstractArray *output)
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

void
VTKMappedVertexArray::GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *output)
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

void
VTKMappedVertexArray::Squeeze()
{
    // noop
}

vtkArrayIterator*
VTKMappedVertexArray::NewIterator()
{
    vtkErrorMacro( << "Not implemented.");
    return NULL;
}

vtkIdType
VTKMappedVertexArray::LookupValue(vtkVariant value)
{
    bool valid = true;
    double val = vtkVariantCast<double>(value, &valid);
    if(valid)
    {
        return this->Lookup(val, 0);
    }
    return -1;
}

void
VTKMappedVertexArray::LookupValue(vtkVariant value, vtkIdList *ids)
{
    bool valid = true;
    double val = vtkVariantCast<double>(value, &valid);
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

vtkVariant
VTKMappedVertexArray::GetVariantValue(vtkIdType idx)
{
    return vtkVariant(this->GetValueReference(idx));
}

void
VTKMappedVertexArray::ClearLookup()
{
    // no-op, no fast lookup implemented.
}

double*
VTKMappedVertexArray::GetTuple(vtkIdType i)
{
    this->TempDoubleArray = (*this->vertexArray)[i];
    return this->TempDoubleArray.data();
}

void
VTKMappedVertexArray::GetTuple(vtkIdType i, double *tuple)
{
    tuple[0] = static_cast<double>((*this->vertexArray)[i][0]);
    tuple[1] = static_cast<double>((*this->vertexArray)[i][1]);
    tuple[2] = static_cast<double>((*this->vertexArray)[i][2]);
}

vtkIdType
VTKMappedVertexArray::LookupTypedValue(double value)
{
    return this->Lookup(value, 0);
}

void
VTKMappedVertexArray::LookupTypedValue(double value, vtkIdList *ids)
{
    ids->Reset();
    vtkIdType index = 0;
    while((index = this->Lookup(value, index)) >= 0)
    {
        ids->InsertNextId(index++);
    }
}

double
VTKMappedVertexArray::GetValue(vtkIdType idx) const
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
            return 0;
    }
}

double&
VTKMappedVertexArray::GetValueReference(vtkIdType idx)
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
            static double dummy(0);
            return dummy;
    }
}

void
VTKMappedVertexArray::GetTypedTuple(vtkIdType tupleId, ValueType *tuple) const
{
  tuple[0] = (*this->vertexArray)[tupleId](0);
  tuple[1] = (*this->vertexArray)[tupleId](1);
  tuple[2] = (*this->vertexArray)[tupleId](2);
}

void
VTKMappedVertexArray::GetTupleValue(vtkIdType tupleId, double *tuple)
{
    tuple[0] = (*this->vertexArray)[tupleId](0);
    tuple[1] = (*this->vertexArray)[tupleId](1);
    tuple[2] = (*this->vertexArray)[tupleId](2);
}

int
VTKMappedVertexArray::Allocate(vtkIdType, vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return 0;
}

int
VTKMappedVertexArray::Resize(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return 0;
}

void
VTKMappedVertexArray::SetNumberOfTuples(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::SetTuple(vtkIdType, vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::SetTuple(vtkIdType, const float *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::SetTuple(vtkIdType, const double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertTuple(vtkIdType, vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertTuple(vtkIdType, const float *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertTuple(vtkIdType, const double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertTuples(vtkIdList *, vtkIdList *, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertTuples(vtkIdType, vtkIdType, vtkIdType, vtkAbstractArray*)
{
    vtkErrorMacro("Read only container.")
    return;
}

vtkIdType
VTKMappedVertexArray::InsertNextTuple(vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

vtkIdType
VTKMappedVertexArray::InsertNextTuple(const float *)
{

    vtkErrorMacro("Read only container.")
    return -1;
}

vtkIdType
VTKMappedVertexArray::InsertNextTuple(const double *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

void
VTKMappedVertexArray::DeepCopy(vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::DeepCopy(vtkDataArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InterpolateTuple(vtkIdType, vtkIdList *, vtkAbstractArray *,
                                    double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InterpolateTuple(vtkIdType, vtkIdType, vtkAbstractArray*,
                                    vtkIdType, vtkAbstractArray*, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::SetVariantValue(vtkIdType, vtkVariant)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::RemoveTuple(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::RemoveFirstTuple()
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::RemoveLastTuple()
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::SetTupleValue(vtkIdType, const double*)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertTupleValue(vtkIdType, const double*)
{
    vtkErrorMacro("Read only container.")
    return;
}

vtkIdType
VTKMappedVertexArray::InsertNextTupleValue(const double *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

void
VTKMappedVertexArray::SetValue(vtkIdType, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

vtkIdType
VTKMappedVertexArray::InsertNextValue(double)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

void
VTKMappedVertexArray::InsertValue(vtkIdType, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

void
VTKMappedVertexArray::InsertVariantValue(vtkIdType, vtkVariant)
{
    vtkErrorMacro("Read only container.")
    return;
}

vtkIdType
VTKMappedVertexArray::Lookup(const double &val, vtkIdType index)
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

} // imstk
