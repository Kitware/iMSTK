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

#include "imstkInternalForceModel.h"

namespace imstk
{
void
InternalForceModel::updateValuesFromMatrix(std::shared_ptr<vega::SparseMatrix> vegaMatrix, double *values)
{
    auto rowLengths = vegaMatrix->GetRowLengths();
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
} // imstk
