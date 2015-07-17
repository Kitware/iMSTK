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

#ifndef SMKEYSFMLINTERFACE_H
#define SMKEYSFMLINTERFACE_H

#include "smKey.h"

#include <SFML/Window/Keyboard.hpp>
#include <map>

namespace smtk {
namespace Event {

static const std::map<int, smKey> sfmlToSmKeyMap = {
    {sf::Keyboard::A, smKey::A},
    {sf::Keyboard::B, smKey::B},
    {sf::Keyboard::C, smKey::C},
    {sf::Keyboard::D, smKey::D},
    {sf::Keyboard::E, smKey::E},
    {sf::Keyboard::F, smKey::F},
    {sf::Keyboard::G, smKey::G},
    {sf::Keyboard::H, smKey::H},
    {sf::Keyboard::I, smKey::I},
    {sf::Keyboard::J, smKey::J},
    {sf::Keyboard::K, smKey::K},
    {sf::Keyboard::L, smKey::L},
    {sf::Keyboard::M, smKey::M},
    {sf::Keyboard::N, smKey::N},
    {sf::Keyboard::O, smKey::O},
    {sf::Keyboard::P, smKey::P},
    {sf::Keyboard::Q, smKey::Q},
    {sf::Keyboard::R, smKey::R},
    {sf::Keyboard::S, smKey::S},
    {sf::Keyboard::T, smKey::T},
    {sf::Keyboard::U, smKey::U},
    {sf::Keyboard::V, smKey::V},
    {sf::Keyboard::W, smKey::W},
    {sf::Keyboard::X, smKey::X},
    {sf::Keyboard::Y, smKey::Y},
    {sf::Keyboard::Z, smKey::Z},
    {sf::Keyboard::Num0, smKey::Num0},
    {sf::Keyboard::Num1, smKey::Num1},
    {sf::Keyboard::Num2, smKey::Num2},
    {sf::Keyboard::Num3, smKey::Num3},
    {sf::Keyboard::Num4, smKey::Num4},
    {sf::Keyboard::Num5, smKey::Num5},
    {sf::Keyboard::Num6, smKey::Num6},
    {sf::Keyboard::Num7, smKey::Num7},
    {sf::Keyboard::Num8, smKey::Num8},
    {sf::Keyboard::Num9, smKey::Num9},
    {sf::Keyboard::Escape, smKey::Escape},
    {sf::Keyboard::LControl, smKey::LControl},
    {sf::Keyboard::LShift, smKey::LShift},
    {sf::Keyboard::LAlt, smKey::LAlt},
    {sf::Keyboard::LSystem, smKey::LSystem},
    {sf::Keyboard::RControl, smKey::RControl},
    {sf::Keyboard::RShift, smKey::RShift},
    {sf::Keyboard::RAlt, smKey::RAlt},
    {sf::Keyboard::RSystem, smKey::RSystem},
    {sf::Keyboard::Menu, smKey::Menu},
    {sf::Keyboard::LBracket, smKey::LBracket},
    {sf::Keyboard::RBracket, smKey::RBracket},
    {sf::Keyboard::SemiColon, smKey::SemiColon},
    {sf::Keyboard::Comma, smKey::Comma},
    {sf::Keyboard::Period, smKey::Period},
    {sf::Keyboard::Quote, smKey::Quote},
    {sf::Keyboard::Slash, smKey::Slash},
    {sf::Keyboard::BackSlash, smKey::BackSlash},
    {sf::Keyboard::Tilde, smKey::Tilde},
    {sf::Keyboard::Equal, smKey::Equal},
    {sf::Keyboard::Dash, smKey::Dash},
    {sf::Keyboard::Space, smKey::Space},
    {sf::Keyboard::Return, smKey::Return},
    {sf::Keyboard::BackSpace, smKey::BackSpace},
    {sf::Keyboard::Tab, smKey::Tab},
    {sf::Keyboard::PageUp, smKey::PageUp},
    {sf::Keyboard::PageDown, smKey::PageDown},
    {sf::Keyboard::End, smKey::End},
    {sf::Keyboard::Home, smKey::Home},
    {sf::Keyboard::Insert, smKey::Insert},
    {sf::Keyboard::Delete, smKey::Delete},
    {sf::Keyboard::Add, smKey::Add},
    {sf::Keyboard::Subtract, smKey::Subtract},
    {sf::Keyboard::Multiply, smKey::Multiply},
    {sf::Keyboard::Divide, smKey::Divide},
    {sf::Keyboard::Left, smKey::Left},
    {sf::Keyboard::Right, smKey::Right},
    {sf::Keyboard::Up, smKey::Up},
    {sf::Keyboard::Down, smKey::Down},
    {sf::Keyboard::Numpad0, smKey::Numpad0},
    {sf::Keyboard::Numpad1, smKey::Numpad1},
    {sf::Keyboard::Numpad2, smKey::Numpad2},
    {sf::Keyboard::Numpad3, smKey::Numpad3},
    {sf::Keyboard::Numpad4, smKey::Numpad4},
    {sf::Keyboard::Numpad5, smKey::Numpad5},
    {sf::Keyboard::Numpad6, smKey::Numpad6},
    {sf::Keyboard::Numpad7, smKey::Numpad7},
    {sf::Keyboard::Numpad8, smKey::Numpad8},
    {sf::Keyboard::Numpad9, smKey::Numpad9},
    {sf::Keyboard::F1, smKey::F1},
    {sf::Keyboard::F2, smKey::F2},
    {sf::Keyboard::F3, smKey::F3},
    {sf::Keyboard::F4, smKey::F4},
    {sf::Keyboard::F5, smKey::F5},
    {sf::Keyboard::F6, smKey::F6},
    {sf::Keyboard::F7, smKey::F7},
    {sf::Keyboard::F8, smKey::F8},
    {sf::Keyboard::F9, smKey::F9},
    {sf::Keyboard::F10, smKey::F10},
    {sf::Keyboard::F11, smKey::F11},
    {sf::Keyboard::F12, smKey::F12},
    {sf::Keyboard::F13, smKey::F13},
    {sf::Keyboard::F14, smKey::F14},
    {sf::Keyboard::F15, smKey::F15},
    {sf::Keyboard::Pause, smKey::Pause},
    {sf::Keyboard::Unknown, smKey::Unknown},
};

inline const smKey& SFMLKeyToSmKey(const int key)
{
    try
    {
        const smKey & out = sfmlToSmKeyMap.at(key)
;        return out;
    }
    catch (std::out_of_range &e)
    {
        std::cout << e.what() << std::endl;
        return sfmlToSmKeyMap.at(sf::Keyboard::Unknown);
    }
    return sfmlToSmKeyMap.at(sf::Keyboard::Unknown);
}

} // Event namespace
} // smtk namespace

#endif