#include<vector>
#include<imstkLogger.h>

template <typename T>
class AccumulationBuffer {
public:
    AccumulationBuffer(size_t capacity) : m_capacity(capacity), m_data(capacity, {}) {

    }

    void push_back(T val) 
    {
        m_sum += val;
        m_data[m_index] = val;
        m_index = (m_index + 1) % m_capacity;
        m_sum -= m_data[m_index];
    }

    T getAverage() const {
        return m_sum / static_cast<T>(m_capacity);
    }

private:
    size_t m_capacity;
    std::vector<T> m_data;
    T m_sum = {};
    size_t m_index = 0;
};