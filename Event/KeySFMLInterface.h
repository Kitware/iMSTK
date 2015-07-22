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

#include "Event/Key.h"

#include <SFML/Window/Keyboard.hpp>
#include <map>

namespace event {

static const std::map<int, Key> sfmlToSmKeyMap = {
    {sf::Keyboard::A, Key::A},
    {sf::Keyboard::B, Key::B},
    {sf::Keyboard::C, Key::C},
    {sf::Keyboard::D, Key::D},
    {sf::Keyboard::E, Key::E},
    {sf::Keyboard::F, Key::F},
    {sf::Keyboard::G, Key::G},
    {sf::Keyboard::H, Key::H},
    {sf::Keyboard::I, Key::I},
    {sf::Keyboard::J, Key::J},
    {sf::Keyboard::K, Key::K},
    {sf::Keyboard::L, Key::L},
    {sf::Keyboard::M, Key::M},
    {sf::Keyboard::N, Key::N},
    {sf::Keyboard::O, Key::O},
    {sf::Keyboard::P, Key::P},
    {sf::Keyboard::Q, Key::Q},
    {sf::Keyboard::R, Key::R},
    {sf::Keyboard::S, Key::S},
    {sf::Keyboard::T, Key::T},
    {sf::Keyboard::U, Key::U},
    {sf::Keyboard::V, Key::V},
    {sf::Keyboard::W, Key::W},
    {sf::Keyboard::X, Key::X},
    {sf::Keyboard::Y, Key::Y},
    {sf::Keyboard::Z, Key::Z},
    {sf::Keyboard::Num0, Key::Num0},
    {sf::Keyboard::Num1, Key::Num1},
    {sf::Keyboard::Num2, Key::Num2},
    {sf::Keyboard::Num3, Key::Num3},
    {sf::Keyboard::Num4, Key::Num4},
    {sf::Keyboard::Num5, Key::Num5},
    {sf::Keyboard::Num6, Key::Num6},
    {sf::Keyboard::Num7, Key::Num7},
    {sf::Keyboard::Num8, Key::Num8},
    {sf::Keyboard::Num9, Key::Num9},
    {sf::Keyboard::Escape, Key::Escape},
    {sf::Keyboard::LControl, Key::LControl},
    {sf::Keyboard::LShift, Key::LShift},
    {sf::Keyboard::LAlt, Key::LAlt},
    {sf::Keyboard::LSystem, Key::LSystem},
    {sf::Keyboard::RControl, Key::RControl},
    {sf::Keyboard::RShift, Key::RShift},
    {sf::Keyboard::RAlt, Key::RAlt},
    {sf::Keyboard::RSystem, Key::RSystem},
    {sf::Keyboard::Menu, Key::Menu},
    {sf::Keyboard::LBracket, Key::LBracket},
    {sf::Keyboard::RBracket, Key::RBracket},
    {sf::Keyboard::SemiColon, Key::SemiColon},
    {sf::Keyboard::Comma, Key::Comma},
    {sf::Keyboard::Period, Key::Period},
    {sf::Keyboard::Quote, Key::Quote},
    {sf::Keyboard::Slash, Key::Slash},
    {sf::Keyboard::BackSlash, Key::BackSlash},
    {sf::Keyboard::Tilde, Key::Tilde},
    {sf::Keyboard::Equal, Key::Equal},
    {sf::Keyboard::Dash, Key::Dash},
    {sf::Keyboard::Space, Key::Space},
    {sf::Keyboard::Return, Key::Return},
    {sf::Keyboard::BackSpace, Key::BackSpace},
    {sf::Keyboard::Tab, Key::Tab},
    {sf::Keyboard::PageUp, Key::PageUp},
    {sf::Keyboard::PageDown, Key::PageDown},
    {sf::Keyboard::End, Key::End},
    {sf::Keyboard::Home, Key::Home},
    {sf::Keyboard::Insert, Key::Insert},
    {sf::Keyboard::Delete, Key::Delete},
    {sf::Keyboard::Add, Key::Add},
    {sf::Keyboard::Subtract, Key::Subtract},
    {sf::Keyboard::Multiply, Key::Multiply},
    {sf::Keyboard::Divide, Key::Divide},
    {sf::Keyboard::Left, Key::Left},
    {sf::Keyboard::Right, Key::Right},
    {sf::Keyboard::Up, Key::Up},
    {sf::Keyboard::Down, Key::Down},
    {sf::Keyboard::Numpad0, Key::Numpad0},
    {sf::Keyboard::Numpad1, Key::Numpad1},
    {sf::Keyboard::Numpad2, Key::Numpad2},
    {sf::Keyboard::Numpad3, Key::Numpad3},
    {sf::Keyboard::Numpad4, Key::Numpad4},
    {sf::Keyboard::Numpad5, Key::Numpad5},
    {sf::Keyboard::Numpad6, Key::Numpad6},
    {sf::Keyboard::Numpad7, Key::Numpad7},
    {sf::Keyboard::Numpad8, Key::Numpad8},
    {sf::Keyboard::Numpad9, Key::Numpad9},
    {sf::Keyboard::F1, Key::F1},
    {sf::Keyboard::F2, Key::F2},
    {sf::Keyboard::F3, Key::F3},
    {sf::Keyboard::F4, Key::F4},
    {sf::Keyboard::F5, Key::F5},
    {sf::Keyboard::F6, Key::F6},
    {sf::Keyboard::F7, Key::F7},
    {sf::Keyboard::F8, Key::F8},
    {sf::Keyboard::F9, Key::F9},
    {sf::Keyboard::F10, Key::F10},
    {sf::Keyboard::F11, Key::F11},
    {sf::Keyboard::F12, Key::F12},
    {sf::Keyboard::F13, Key::F13},
    {sf::Keyboard::F14, Key::F14},
    {sf::Keyboard::F15, Key::F15},
    {sf::Keyboard::Pause, Key::Pause},
    {sf::Keyboard::Unknown, Key::Unknown},
};

inline const Key& SFMLKeyToSmKey(const int key)
{
    try
    {
        const Key & out = sfmlToSmKeyMap.at(key);
        return out;
    }
    catch (std::out_of_range &e)
    {
        std::cout << e.what() << std::endl;
        return sfmlToSmKeyMap.at(sf::Keyboard::Unknown);
    }
    return sfmlToSmKeyMap.at(sf::Keyboard::Unknown);
}

} // event namespace

#endif
