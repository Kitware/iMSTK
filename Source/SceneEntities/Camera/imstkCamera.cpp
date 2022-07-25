/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"

namespace imstk
{
void
Camera::print()
{
    std::cout << "CamPos: " << m_position[0] << ", " << m_position[1] << ", " << m_position[2] << std::endl;
    std::cout << "FocalPoint: " << m_focalPoint[0] << ", " << m_focalPoint[1] << ", " << m_focalPoint[2] << std::endl;
    std::cout << "Up: " << m_viewUp[0] << ", " << m_viewUp[1] << ", " << m_viewUp[2] << std::endl;
}
} // namespace imstk