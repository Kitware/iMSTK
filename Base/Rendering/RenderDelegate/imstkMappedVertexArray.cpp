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

#include "imstkMappedVertexArray.h"

#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkVariant.h"
#include "vtkVariantCast.h"

namespace imstk {

vtkStandardNewMacro(MappedVertexArray);

//------------------------------------------------------------------------------
void
MappedVertexArray::PrintSelf(ostream &os, vtkIndent indent)
{
    this->MappedVertexArray::Superclass::PrintSelf(
        os, indent);

    os << indent << "vertexArray : " << this->vertexArray << std::endl;
    os << "TempDoubleArray : " << this->TempDoubleArray << std::endl;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetVertexArray(std::vector<Vec3d>& vertices)
{
    this->Initialize();
    this->NumberOfComponents = 3;
    this->Size = this->NumberOfComponents * vertices.size();
    this->MaxId = this->Size - 1;
    this->vertexArray = &vertices;
    this->Modified();
}

//------------------------------------------------------------------------------
void
MappedVertexArray::Initialize()
{
    this->vertexArray = nullptr;
    this->MaxId = -1;
    this->Size = 0;
    this->NumberOfComponents = 1;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::GetTuples(vtkIdList *ptIds, vtkAbstractArray *output)
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
void
MappedVertexArray::GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *output)
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
void
MappedVertexArray::Squeeze()
{
    // noop
}

//------------------------------------------------------------------------------
vtkArrayIterator*
MappedVertexArray::NewIterator()
{
    vtkErrorMacro( << "Not implemented.");
    return NULL;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::LookupValue(vtkVariant value)
{
    bool valid = true;
    double val = vtkVariantCast<double>(value, &valid);
    if(valid)
    {
        return this->Lookup(val, 0);
    }
    return -1;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::LookupValue(vtkVariant value, vtkIdList *ids)
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

//------------------------------------------------------------------------------
vtkVariant
MappedVertexArray::GetVariantValue(vtkIdType idx)
{
    return vtkVariant(this->GetValueReference(idx));
}

//------------------------------------------------------------------------------
void
MappedVertexArray::ClearLookup()
{
    // no-op, no fast lookup implemented.
}

//------------------------------------------------------------------------------
double*
MappedVertexArray::GetTuple(vtkIdType i)
{
    this->TempDoubleArray = (*this->vertexArray)[i];
    return this->TempDoubleArray.data();
}

//------------------------------------------------------------------------------
void
MappedVertexArray::GetTuple(vtkIdType i, double *tuple)
{
    tuple[0] = static_cast<double>((*this->vertexArray)[i][0]);
    tuple[1] = static_cast<double>((*this->vertexArray)[i][1]);
    tuple[2] = static_cast<double>((*this->vertexArray)[i][2]);
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::LookupTypedValue(double value)
{
    return this->Lookup(value, 0);
}

//------------------------------------------------------------------------------
void
MappedVertexArray::LookupTypedValue(double value, vtkIdList *ids)
{
    ids->Reset();
    vtkIdType index = 0;
    while((index = this->Lookup(value, index)) >= 0)
    {
        ids->InsertNextId(index++);
    }
}

//------------------------------------------------------------------------------
double
MappedVertexArray::GetValue(vtkIdType idx)
{
    return this->GetValueReference(idx);
}

//------------------------------------------------------------------------------
double&
MappedVertexArray::GetValueReference(vtkIdType idx)
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

//------------------------------------------------------------------------------
void
MappedVertexArray::GetTupleValue(vtkIdType tupleId, double *tuple)
{
    tuple[0] = (*this->vertexArray)[tupleId](0);
    tuple[1] = (*this->vertexArray)[tupleId](1);
    tuple[2] = (*this->vertexArray)[tupleId](2);
}

//------------------------------------------------------------------------------
int
MappedVertexArray::Allocate(vtkIdType, vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return 0;
}

//------------------------------------------------------------------------------
int
MappedVertexArray::Resize(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return 0;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetNumberOfTuples(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetTuple(vtkIdType, vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetTuple(vtkIdType, const float *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetTuple(vtkIdType, const double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertTuple(vtkIdType, vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertTuple(vtkIdType, const float *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertTuple(vtkIdType, const double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertTuples(vtkIdList *, vtkIdList *, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertTuples(vtkIdType,vtkIdType,vtkIdType,vtkAbstractArray*)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::InsertNextTuple(vtkIdType, vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::InsertNextTuple(const float *)
{

    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::InsertNextTuple(const double *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::DeepCopy(vtkAbstractArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::DeepCopy(vtkDataArray *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InterpolateTuple(vtkIdType, vtkIdList *, vtkAbstractArray *,
                                    double *)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InterpolateTuple(vtkIdType, vtkIdType, vtkAbstractArray*,
                                    vtkIdType, vtkAbstractArray*, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetVariantValue(vtkIdType, vtkVariant)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::RemoveTuple(vtkIdType)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::RemoveFirstTuple()
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::RemoveLastTuple()
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetTupleValue(vtkIdType, const double*)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertTupleValue(vtkIdType, const double*)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::InsertNextTupleValue(const double *)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::SetValue(vtkIdType, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::InsertNextValue(double)
{
    vtkErrorMacro("Read only container.")
    return -1;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertValue(vtkIdType, double)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
void
MappedVertexArray::InsertVariantValue(vtkIdType, vtkVariant)
{
    vtkErrorMacro("Read only container.")
    return;
}

//------------------------------------------------------------------------------
vtkIdType
MappedVertexArray::Lookup(const double &val, vtkIdType index)
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
