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

#include "imstkVTKVolumeRenderDelegate.h"

class vtkImageData;

namespace imstk
{
class AbstractDataArray;

///
/// \class VTKImageDataRenderDelegate
///
/// \brief Render delegate to do to volume rendering using VTK
///
class VTKImageDataRenderDelegate : public VTKVolumeRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    explicit VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Destructor
    ///
    virtual ~VTKImageDataRenderDelegate() override = default;

    ///
    /// \brief Update render delegate source based on the internal data
    ///
    void processEvents() override;

protected:
    ///
    /// \brief Callback for when the image is modified
    ///
    void imageDataModified(Event* e);

    ///
    /// \brief Callback for when the image scalars are modified
    ///
    void imageScalarsModified(Event* e);

    std::shared_ptr<AbstractDataArray> m_scalarArray;
    vtkSmartPointer<vtkImageData>      imageDataVtk;
};
} // imstk
