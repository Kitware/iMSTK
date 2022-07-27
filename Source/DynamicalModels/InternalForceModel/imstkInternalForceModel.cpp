/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkInternalForceModel.h"

namespace imstk
{
void
InternalForceModel::updateValuesFromMatrix(std::shared_ptr<vega::SparseMatrix> vegaMatrix, double* values)
{
    auto rowLengths    = vegaMatrix->GetRowLengths();
    auto nonZeroValues = vegaMatrix->GetEntries();

    // Flatten the internal non-zeros value array and store it in values.
    int offset = 0;

    for (int row = 0, end = vegaMatrix->GetNumRows(); row < end; ++row)
    {
        /// This operation should not add new values to the array since the matrices
        /// structures should remain the same. It just replaces the values in the array.
        for (int j = 0, end_j = rowLengths[row]; j < end_j; ++j)
        {
            values[j + offset] = nonZeroValues[row][j];
        }

        offset += rowLengths[row];
    }
}

void
InternalForceModel::getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix)
{
    this->getInternalForce(u, internalForce);
    this->getTangentStiffnessMatrix(u, tangentStiffnessMatrix);
}
} // namespace imstk