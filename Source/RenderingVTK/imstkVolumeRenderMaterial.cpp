/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVolumeRenderMaterial.h"

#include <vtkVolumeProperty.h>

namespace imstk
{
VolumeRenderMaterial::VolumeRenderMaterial() : m_property(vtkSmartPointer<vtkVolumeProperty>::New())
{
}

vtkSmartPointer<vtkVolumeProperty>
VolumeRenderMaterial::getVolumeProperty()
{
    return this->m_property;
}
} // namespace imstk
