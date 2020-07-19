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

#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

namespace imstk
{
namespace expiremental
{
///
/// \class Signal
///
/// \brief Signals call N function pointers when emit is called
///
template<typename ... Args>
class Signal
{
public:
    template<typename T>
    int connect(T* instance, void (T::* func)(Args...))
    {
        return connect([ = ](Args... args) { (instance->*func)(args ...); });
    }

    template<typename T>
    int connect(T* instance, void (T::* func)(Args...) const)
    {
        return connect([ = ](Args... args) { (instance->*func)(args ...); });
    }

    unsigned int connect(const std::function<void(Args...)>& slot)
    {
        const unsigned int id = handleIter;
        slots.insert(std::make_pair(++handleIter, slot));
        return id;
    }

    void disconnect(int id) { slots.erase(id); }

    void disconnectAll() { slots.clear(); }

    void emit(Args... p)
    {
        for (std::unordered_map<unsigned int, std::function<void(Args...)>>::iterator it = slots.begin(); it != slots.end(); it++)
        {
            it->second(p ...);
        }
    }

private:
    std::unordered_map<unsigned int, std::function<void(Args...)>> slots;
    unsigned int handleIter = 0;
};
}
}