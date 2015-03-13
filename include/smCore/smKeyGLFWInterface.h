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

#ifndef SMKEYGLFWINTERFACE_H
#define SMKEYGLFWINTERFACE_H

// STL includes
#include <map>

// SimMedTK includes
#include "smCore/smKey.h"

#include <GLFW/glfw3.h>

static const std::map<int, smKey> glfwToSmKeyMap = {
    {GLFW_KEY_A, smKey::A},
    {GLFW_KEY_B, smKey::B},
    {GLFW_KEY_C, smKey::C},
    {GLFW_KEY_D, smKey::D},
    {GLFW_KEY_E, smKey::E},
    {GLFW_KEY_F, smKey::F},
    {GLFW_KEY_G, smKey::G},
    {GLFW_KEY_H, smKey::H},
    {GLFW_KEY_I, smKey::I},
    {GLFW_KEY_J, smKey::J},
    {GLFW_KEY_K, smKey::K},
    {GLFW_KEY_L, smKey::L},
    {GLFW_KEY_M, smKey::M},
    {GLFW_KEY_N, smKey::N},
    {GLFW_KEY_O, smKey::O},
    {GLFW_KEY_P, smKey::P},
    {GLFW_KEY_Q, smKey::Q},
    {GLFW_KEY_R, smKey::R},
    {GLFW_KEY_S, smKey::S},
    {GLFW_KEY_T, smKey::T},
    {GLFW_KEY_U, smKey::U},
    {GLFW_KEY_V, smKey::V},
    {GLFW_KEY_W, smKey::W},
    {GLFW_KEY_X, smKey::X},
    {GLFW_KEY_Y, smKey::Y},
    {GLFW_KEY_Z, smKey::Z},
    {GLFW_KEY_0, smKey::Num0},
    {GLFW_KEY_1, smKey::Num1},
    {GLFW_KEY_2, smKey::Num2},
    {GLFW_KEY_3, smKey::Num3},
    {GLFW_KEY_4, smKey::Num4},
    {GLFW_KEY_5, smKey::Num5},
    {GLFW_KEY_6, smKey::Num6},
    {GLFW_KEY_7, smKey::Num7},
    {GLFW_KEY_8, smKey::Num8},
    {GLFW_KEY_9, smKey::Num9},
    {GLFW_KEY_ESCAPE, smKey::Escape},
    {GLFW_KEY_LEFT_CONTROL, smKey::LControl},
    {GLFW_KEY_LEFT_SHIFT, smKey::LShift},
    {GLFW_KEY_LEFT_ALT, smKey::LAlt},
    {GLFW_KEY_LEFT_SUPER, smKey::LSystem},
    {GLFW_KEY_RIGHT_CONTROL, smKey::RControl},
    {GLFW_KEY_RIGHT_SHIFT, smKey::RShift},
    {GLFW_KEY_RIGHT_ALT, smKey::RAlt},
    {GLFW_KEY_RIGHT_SUPER, smKey::RSystem},
    {GLFW_KEY_MENU, smKey::Menu},
    {GLFW_KEY_LEFT_BRACKET, smKey::LBracket},
    {GLFW_KEY_RIGHT_BRACKET, smKey::RBracket},
    {GLFW_KEY_SEMICOLON, smKey::SemiColon},
    {GLFW_KEY_COMMA, smKey::Comma},
    {GLFW_KEY_PERIOD, smKey::Period},
    {GLFW_KEY_APOSTROPHE, smKey::Quote},
    {GLFW_KEY_SLASH, smKey::Slash},
    {GLFW_KEY_BACKSLASH, smKey::BackSlash},
    {GLFW_KEY_EQUAL, smKey::Equal},
    {GLFW_KEY_MINUS, smKey::Dash},
    {GLFW_KEY_SPACE, smKey::Space},
    {GLFW_KEY_ENTER, smKey::Return},
    {GLFW_KEY_BACKSPACE, smKey::BackSpace},
    {GLFW_KEY_TAB, smKey::Tab},
    {GLFW_KEY_PAGE_UP, smKey::PageUp},
    {GLFW_KEY_PAGE_DOWN, smKey::PageDown},
    {GLFW_KEY_END, smKey::End},
    {GLFW_KEY_HOME, smKey::Home},
    {GLFW_KEY_INSERT, smKey::Insert},
    {GLFW_KEY_DELETE, smKey::Delete},
    {GLFW_KEY_KP_ADD, smKey::Add},
    {GLFW_KEY_KP_SUBTRACT, smKey::Subtract},
    {GLFW_KEY_KP_MULTIPLY, smKey::Multiply},
    {GLFW_KEY_KP_DIVIDE, smKey::Divide},
    {GLFW_KEY_LEFT, smKey::Left},
    {GLFW_KEY_RIGHT, smKey::Right},
    {GLFW_KEY_UP, smKey::Up},
    {GLFW_KEY_DOWN, smKey::Down},
    {GLFW_KEY_KP_0, smKey::Numpad0},
    {GLFW_KEY_KP_1, smKey::Numpad1},
    {GLFW_KEY_KP_2, smKey::Numpad2},
    {GLFW_KEY_KP_3, smKey::Numpad3},
    {GLFW_KEY_KP_4, smKey::Numpad4},
    {GLFW_KEY_KP_5, smKey::Numpad5},
    {GLFW_KEY_KP_6, smKey::Numpad6},
    {GLFW_KEY_KP_7, smKey::Numpad7},
    {GLFW_KEY_KP_8, smKey::Numpad8},
    {GLFW_KEY_KP_9, smKey::Numpad9},
    {GLFW_KEY_F1, smKey::F1},
    {GLFW_KEY_F2, smKey::F2},
    {GLFW_KEY_F3, smKey::F3},
    {GLFW_KEY_F4, smKey::F4},
    {GLFW_KEY_F5, smKey::F5},
    {GLFW_KEY_F6, smKey::F6},
    {GLFW_KEY_F7, smKey::F7},
    {GLFW_KEY_F8, smKey::F8},
    {GLFW_KEY_F9, smKey::F9},
    {GLFW_KEY_F10, smKey::F10},
    {GLFW_KEY_F11, smKey::F11},
    {GLFW_KEY_F12, smKey::F12},
    {GLFW_KEY_F13, smKey::F13},
    {GLFW_KEY_F14, smKey::F14},
    {GLFW_KEY_F15, smKey::F15},
    {GLFW_KEY_PAUSE, smKey::Pause},
    {GLFW_KEY_GRAVE_ACCENT, smKey::Backtick},
};

inline smKey GLFWKeyToSmKey(int key)
{
    return glfwToSmKeyMap.at(key);
}

#endif
