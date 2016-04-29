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

#ifndef imstkMappedVertexArray_h
#define imstkMappedVertexArray_h

#include "vtkMappedDataArray.h"
#include "vtkObjectFactory.h" // for vtkStandardNewMacro

// iMSTK includes
#include "imstkMath.h"

// STL includes
#include <array>

namespace imstk {
class MappedVertexArray: public vtkMappedDataArray<double>
{
public:
    static MappedVertexArray *New();
    vtkTypeMacro(MappedVertexArray,vtkMappedDataArray);
    virtual void PrintSelf(ostream &os, vtkIndent indent);

    // Description:
    // Set the raw scalar arrays for the coordinate set. This class takes
    // ownership of the arrays and deletes them with delete[].
    void SetVertexArray(std::vector<Vec3d> &vertices);

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
    vtkIdType LookupTypedValue(double value);
    void LookupTypedValue(double value, vtkIdList *ids);
    double GetValue(vtkIdType idx);
    double& GetValueReference(vtkIdType idx);
    void GetTupleValue(vtkIdType idx, double *t);

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
    void SetTupleValue(vtkIdType i, const double *t);
    void InsertTupleValue(vtkIdType i, const double *t);
    vtkIdType InsertNextTupleValue(const double *t);
    void SetValue(vtkIdType idx, double value);
    vtkIdType InsertNextValue(double v);
    void InsertValue(vtkIdType idx, double v);
    void InsertVariantValue(vtkIdType idx, vtkVariant value);

protected:
    MappedVertexArray() {}
    ~MappedVertexArray() {}

    std::vector<Vec3d> *vertexArray;

private:
    MappedVertexArray(const MappedVertexArray &); // Not implemented.
    void operator=(const MappedVertexArray &); // Not implemented.

    vtkIdType Lookup(const double &val, vtkIdType startIndex);
    Vec3d TempDoubleArray;
};
}

#endif // ifndef imstkMappedVertexArray_h
