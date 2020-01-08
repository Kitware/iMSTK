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

#include "imstkVTKImageDataRenderDelegate.h"

#include "vtkTrivialProducer.h"
#include "vtkVolumeProperty.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkGPUVolumeRayCastMapper.h"

namespace imstk
{
VTKImageDataRenderDelegate::VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    m_visualModel = visualModel;

    auto imageData = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());
    if (imageData->getData())
    {
        auto tp = vtkSmartPointer<vtkTrivialProducer>::New();
        tp->SetOutput(imageData->getData());
        this->setUpMapper(tp->GetOutputPort(), false,
                          m_visualModel->getRenderMaterial());
    }

    vtkNew<vtkVolumeProperty>        prop;
    vtkNew<vtkColorTransferFunction> ctf;
    ctf->AddRGBPoint(-3024, 0, 0, 0);
    ctf->AddRGBPoint(-568.63, 203 / 255., 132 / 255., 66 / 255.);
    ctf->AddRGBPoint(18.28, 216 / 255., 101 / 255., 79 / 255.);
    ctf->AddRGBPoint(408.22, 192 / 255., 192 / 255., 192 / 255.);
    ctf->AddRGBPoint(2278.94, 235 / 255., 236 / 255., 211 / 255.);
    ctf->AddRGBPoint(2835.87, 1.0, 250 / 255., 220 / 255.);
    ctf->AddRGBPoint(3592.73, 242 / 255., 244 / 255., 232 / 255.);
    vtkNew<vtkPiecewiseFunction> pf;
    pf->AddPoint(-3024, 0.0);
    pf->AddPoint(-291.5, 0.0);
    pf->AddPoint(-191.56, 0.11);
    pf->AddPoint(136.89, 0.3);
    pf->AddPoint(579.58, 0.47);
    pf->AddPoint(1079.39, 0.68);
    pf->AddPoint(1807.69, 0.84);
    pf->AddPoint(2635.95, 0.91);
    pf->AddPoint(3262.47, 0.90);
    vtkNew<vtkPiecewiseFunction> gf;
    gf->AddPoint(-20, 0);
    gf->AddPoint(3592, 1);

    prop->SetColor(ctf);
    prop->SetScalarOpacity(pf);
    prop->SetGradientOpacity(gf);
    prop->SetShade(1);
    prop->SetAmbient(0.2);
    prop->SetDiffuse(1.0);
    prop->SetSpecular(0.0);
    prop->SetSpecularPower(1.0);
    prop->SetInterpolationTypeToLinear();
    this->m_volume->SetProperty(prop);
}

void
VTKImageDataRenderDelegate::updateDataSource()
{
    auto imageData = std::static_pointer_cast<ImageData>(m_visualModel->getGeometry());

    if (!imageData->m_dataModified)
    {
        return;
    }

    // TODO: Any transforms/modifications?
    imageData->m_dataModified = false;
}
} // imstk
