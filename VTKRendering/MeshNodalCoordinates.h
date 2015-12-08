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


#ifndef MESHNODALCOORDINATES_H
#define MESHNODALCOORDINATES_H

#include "vtkMappedDataArray.h"

#include "vtkTypeTemplate.h" // For templated vtkObject API
#include "vtkObjectFactory.h" // for vtkStandardNewMacro

// SimMedTK includes
#include "Core/Vector.h"

// STL includes
#include <array>

template <class Scalar>
class MeshNodalCoordinates:
    public vtkTypeTemplate<MeshNodalCoordinates<Scalar>, vtkMappedDataArray<Scalar>>
{
public:
    vtkMappedDataArrayNewInstanceMacro(MeshNodalCoordinates<Scalar>)
    static MeshNodalCoordinates *New();
    virtual void PrintSelf(ostream &os, vtkIndent indent);

    // Description:
    // Set the raw scalar arrays for the coordinate set. This class takes
    // ownership of the arrays and deletes them with delete[].
    void SetVertexArray(std::vector<Vector3<Scalar>> &vertices);

    // Reimplemented virtuals -- see superclasses for descriptions:
    void Initialize();
    void GetTuples(vtkIdList *ptIds, vtkAbstractArray *output);
    void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *output);
    void Squeeze();
    vtkArrayIterator *NewIterator();
    vtkIdType LookupValue(vtkVariant value);
    void LookupValue(vtkVariant value, vtkIdList *ids);
    vtkVariant GetVariantValue(vtkIdType idx);
    void ClearLookup();
    double* GetTuple(vtkIdType i);
    void GetTuple(vtkIdType i, double *tuple);
    vtkIdType LookupTypedValue(Scalar value);
    void LookupTypedValue(Scalar value, vtkIdList *ids);
    Scalar GetValue(vtkIdType idx);
    Scalar& GetValueReference(vtkIdType idx);
    void GetTupleValue(vtkIdType idx, Scalar *t);

    // Description:
    // This container is read only -- this method does nothing but print a
    // warning.
    int Allocate(vtkIdType sz, vtkIdType ext);
    int Resize(vtkIdType numTuples);
    void SetNumberOfTuples(vtkIdType number);
    void SetTuple(vtkIdType i, vtkIdType j, vtkAbstractArray *source);
    void SetTuple(vtkIdType i, const float *source);
    void SetTuple(vtkIdType i, const double *source);
    void InsertTuple(vtkIdType i, vtkIdType j, vtkAbstractArray *source);
    void InsertTuple(vtkIdType i, const float *source);
    void InsertTuple(vtkIdType i, const double *source);
    void InsertTuples(vtkIdList *dstIds, vtkIdList *srcIds,
                      vtkAbstractArray *source);
    void InsertTuples(vtkIdType dstStart, vtkIdType n, vtkIdType srcStart,
                      vtkAbstractArray* source);
    vtkIdType InsertNextTuple(vtkIdType j, vtkAbstractArray *source);
    vtkIdType InsertNextTuple(const float *source);
    vtkIdType InsertNextTuple(const double *source);
    void DeepCopy(vtkAbstractArray *aa);
    void DeepCopy(vtkDataArray *da);
    void InterpolateTuple(vtkIdType i, vtkIdList *ptIndices,
                          vtkAbstractArray* source,  double* weights);
    void InterpolateTuple(vtkIdType i, vtkIdType id1, vtkAbstractArray *source1,
                          vtkIdType id2, vtkAbstractArray *source2, double t);
    void SetVariantValue(vtkIdType idx, vtkVariant value);
    void RemoveTuple(vtkIdType id);
    void RemoveFirstTuple();
    void RemoveLastTuple();
    void SetTupleValue(vtkIdType i, const Scalar *t);
    void InsertTupleValue(vtkIdType i, const Scalar *t);
    vtkIdType InsertNextTupleValue(const Scalar *t);
    void SetValue(vtkIdType idx, Scalar value);
    vtkIdType InsertNextValue(Scalar v);
    void InsertValue(vtkIdType idx, Scalar v);
    void InsertVariantValue(vtkIdType idx, vtkVariant value);

protected:
    MeshNodalCoordinates();
    ~MeshNodalCoordinates();

    std::vector<Vector3<Scalar>> *vertexArray;

private:
    MeshNodalCoordinates(const MeshNodalCoordinates &); // Not implemented.
    void operator=(const MeshNodalCoordinates &); // Not implemented.

    vtkIdType Lookup(const Scalar &val, vtkIdType startIndex);
    core::Vec3d TempDoubleArray;
};

#include "MeshNodalCoordinates.txx"

#endif // MESHNODALCOORDINATES_H
