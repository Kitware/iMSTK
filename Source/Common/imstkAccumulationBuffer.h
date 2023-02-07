/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/
#include <vector>
#include <imstkLogger.h>

/// \brief Class to accumulate values for determining a rolling average
/// note that the buffer will be filled with `0` at initialization,
/// therefore the average will ramp up slowly as the buffer is filled
template<typename T>
class AccumulationBuffer
{
public:
    explicit AccumulationBuffer(size_t capacity = 1024) : m_capacity(capacity), m_data(capacity, {})
    {
    }

    /// \return the capacity of the buffer
    size_t getCapacity() const
    {
        return m_capacity;
    }

    /// Pushes the value onto the buffer
    void pushBack(T val)
    {
        m_sum += val;
        m_sum -= m_data[m_index];
        m_data[m_index] = val;
        m_index = (m_index + 1) % m_capacity;
    }

    /// \return the average of all values in the buffer
    T getAverage() const
    {
        return m_sum / static_cast<T>(m_capacity);
    }

private:
    size_t m_capacity;
    std::vector<T> m_data;
    T      m_sum   = {}; ///< Sum of all the values in the buffer
    size_t m_index = 0;  ///< Index of the current value
};