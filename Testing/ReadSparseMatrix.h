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

#ifndef READ_SPARSE_MATRIX_H
#define READ_SPARSE_MATRIX_H

#include<iostream>
#include<fstream>
#include<Eigen/Core>
#include<Eigen/Sparse>

//
// @brief Reads a sparse matrix form the Matrix market.
//
// @param fileName Matriz filename
// @param A Read matrix
// @return Eigen::SparseMatrix< T, opt >& A
//
template<typename T, int opt>
Eigen::SparseMatrix<T,opt> &ReadSparseMatrix(const std::string &fileName, Eigen::SparseMatrix<T,opt> &A)
{
  std::ifstream fStream(fileName.c_str());
  size_t numCols = 0, numRows = 0, numNonZeros = 0;
  std::vector<Eigen::Triplet<T>> tripletList;
  if(fStream.is_open())
  {
    fStream.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
    fStream.precision(16);
    fStream >> numRows >> numCols >> numNonZeros;

//     std::cout << "Matrix " << fileName << std::endl;
//     std::cout << "Matrix size: cols [" << numCols << "]; rows [" << numRows << "]; nonZeros [" << numNonZeros << "]" << std::endl;
    A.resize(numRows,numCols);
    A.reserve(2*numNonZeros);
    tripletList.reserve(2*numNonZeros);
    for(size_t n = 0; n < numNonZeros; ++n)
    {
      size_t i, j;
      T aij;
      fStream >> i >> j >> aij;
      tripletList.emplace_back(i-1,j-1,aij);
      tripletList.emplace_back(j-1,i-1,aij);
    }
    A.setFromTriplets(tripletList.begin(),tripletList.end());
  }
  fStream.close();
  return A;
}

#endif
