/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <atomic>
#include "imstkMath.h"

namespace imstk
{
namespace ParallelUtils
{
///
/// \brief Perform an atomic operation: target = f(target, operand)
///
template<class T, class Function>
void
atomicOp(T& target, const T operand, Function&& f)
{
    std::atomic<T>& tgt = *(static_cast<std::atomic<T>*>(&target));

    T cur_val = target;
    T new_val;
    do
    {
        new_val = f(cur_val, operand);
    }
    while (!tgt.compare_exchange_weak(cur_val, new_val));
}

///
/// \brief Atomic addition for scalar numbers: target = target + operand
///
template<class T>
void
atomicAdd(T& target, const T operand)
{
    atomicOp(target, operand, [](T a, T b) { return a + b; });
}

///
/// \brief Atomic subtraction for scalar numbers: target = target - operand
///
template<class T>
void
atomicSubtract(T& target, const T operand)
{
    atomicOp(target, operand, [](T a, T b) { return a - b; });
}

///
/// \brief Atomic multiplication for scalar numbers: target = target * operand
///
template<class T>
void
atomicMultiply(T& target, const T operand)
{
    atomicOp(target, operand, [](T a, T b) { return a * b; });
}

///
/// \brief Atomic division for scalar numbers: target = target / operand
///
template<class T>
void
atomicDivide(T& target, const T operand)
{
    atomicOp(target, operand, [](T a, T b) { return a / b; });
}

///
/// \brief Atomic addition for two vectors: target = target + operand
///
template<class T, int N>
void
atomicAdd(Eigen::Matrix<T, N, 1>& target, const Eigen::Matrix<T, N, 1>& operand)
{
    for (int i = 0; i < N; ++i)
    {
        atomicAdd(target[i], operand[i]);
    }
}

///
/// \brief Atomic subtraction for two vectors: target = target - operand
///
template<class T, int N>
void
atomicSubtract(Eigen::Matrix<T, N, 1>& target, const Eigen::Matrix<T, N, 1>& operand)
{
    for (int i = 0; i < N; ++i)
    {
        atomicSubtract(target[i], operand[i]);
    }
}

///
/// \brief Atomic multiplication for a vector and a scalar number: target = target * operand
///
template<class T, int N>
void
atomicMultiply(Eigen::Matrix<T, N, 1>& target, const T operand)
{
    for (int i = 0; i < N; ++i)
    {
        atomicMultiply(target[i], operand);
    }
}

///
/// \brief Atomic division for a vector and a scalar number: target = target / operand
///
template<class T, int N>
void
atomicDivide(Eigen::Matrix<T, N, 1>& target, const T operand)
{
    for (int i = 0; i < N; ++i)
    {
        atomicDivide(target[i], operand);
    }
}
} // end namespace namespace ParallelUtils
} // end namespace imstk
