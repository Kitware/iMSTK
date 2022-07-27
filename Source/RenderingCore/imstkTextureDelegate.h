/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkEventObject.h"

#include <memory>

namespace imstk
{
class Texture;

///
/// \class TextureDelegate
///
/// \brief iMSTK texture delegate abstract class
///
class TextureDelegate : public EventObject
{
public:
    virtual ~TextureDelegate() = default;

protected:
    TextureDelegate() = default;
};
} // namespace imstk
