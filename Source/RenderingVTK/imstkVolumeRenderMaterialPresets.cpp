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

#include "imstkVolumeRenderMaterialPresets.h"

// imstk
#include "imstkVolumeRenderMaterial.h"

// vtk
#include <vtkVolumeProperty.h>
#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

namespace imstk
{
std::shared_ptr<VolumeRenderMaterial>
VolumeRenderMaterialPresets::getPreset(const int p)
{
    auto mat     = std::make_shared<VolumeRenderMaterial>();
    auto volProp = mat->getVolumeProperty();
    auto ctf     = vtkSmartPointer<vtkColorTransferFunction>::New();
    volProp->SetColor(ctf);
    auto of = vtkSmartPointer<vtkPiecewiseFunction>::New();
    volProp->SetScalarOpacity(of);
    auto gf = vtkSmartPointer<vtkPiecewiseFunction>::New();
    volProp->SetGradientOpacity(gf);

    switch (p)
    {
    case VolumeRenderMaterialPresets::CT_AAA:
    {
        // color
        ctf->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
        ctf->AddRGBPoint(143.556, 0.615686, 0.356863, 0.184314);
        ctf->AddRGBPoint(166.222, 0.882353, 0.603922, 0.290196);
        ctf->AddRGBPoint(214.389, 1, 1, 1);
        ctf->AddRGBPoint(419.736, 1, 0.937033, 0.954531);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(143.556, 0);
        of->AddPoint(166.222, 0.686275);
        of->AddPoint(214.389, 0.696078);
        of->AddPoint(419.736, 0.833333);
        of->AddPoint(3071, 0.803922);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_AAA2:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(129.542, 0.54902, 0.25098, 0.14902);
        ctf->AddRGBPoint(145.244, 0.6, 0.627451, 0.843137);
        ctf->AddRGBPoint(157.02, 0.890196, 0.47451, 0.6);
        ctf->AddRGBPoint(169.918, 0.992157, 0.870588, 0.392157);
        ctf->AddRGBPoint(395.575, 1, 0.886275, 0.658824);
        ctf->AddRGBPoint(1578.73, 1, 0.829256, 0.957922);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(129.542, 0);
        of->AddPoint(145.244, 0.166667);
        of->AddPoint(157.02, 0.5);
        of->AddPoint(169.918, 0.627451);
        of->AddPoint(395.575, 0.8125);
        of->AddPoint(1578.73, 0.8125);
        of->AddPoint(3071, 0.8125);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_BONE:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(-16.4458, 0.729412, 0.254902, 0.301961);
        ctf->AddRGBPoint(641.385, 0.905882, 0.815686, 0.552941);
        ctf->AddRGBPoint(3071, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-16.4458, 0);
        of->AddPoint(641.385, 0.715686);
        of->AddPoint(3071, 0.705882);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_BONES:
    {
        // color
        ctf->AddRGBPoint(-1000, 0.3, 0.3, 1);
        ctf->AddRGBPoint(-488, 0.3, 1, 0.3);
        ctf->AddRGBPoint(463.28, 1, 0, 0);
        ctf->AddRGBPoint(659.15, 1, 0.912535, 0.0374849);
        ctf->AddRGBPoint(953, 1, 0.3, 0.3);

        // scalar opacity
        of->AddPoint(152.19, 0);
        of->AddPoint(278.93, 0.190476);
        of->AddPoint(952, 0.2);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CARDIAC:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(-77.6875, 0.54902, 0.25098, 0.14902);
        ctf->AddRGBPoint(94.9518, 0.882353, 0.603922, 0.290196);
        ctf->AddRGBPoint(179.052, 1, 0.937033, 0.954531);
        ctf->AddRGBPoint(260.439, 0.615686, 0, 0);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-77.6875, 0);
        of->AddPoint(94.9518, 0.285714);
        of->AddPoint(179.052, 0.553571);
        of->AddPoint(260.439, 0.848214);
        of->AddPoint(3071, 0.875);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CARDIAC2:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(42.8964, 0.54902, 0.25098, 0.14902);
        ctf->AddRGBPoint(163.488, 0.917647, 0.639216, 0.0588235);
        ctf->AddRGBPoint(277.642, 1, 0.878431, 0.623529);
        ctf->AddRGBPoint(1587, 1, 1, 1);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(42.8964, 0);
        of->AddPoint(163.488, 0.428571);
        of->AddPoint(277.642, 0.776786);
        of->AddPoint(1587, 0.754902);
        of->AddPoint(3071, 0.754902);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CARDIAC3:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(-86.9767, 0, 0.25098, 1);
        ctf->AddRGBPoint(45.3791, 1, 0, 0);
        ctf->AddRGBPoint(139.919, 1, 0.894893, 0.894893);
        ctf->AddRGBPoint(347.907, 1, 1, 0.25098);
        ctf->AddRGBPoint(1224.16, 1, 1, 1);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-86.9767, 0);
        of->AddPoint(45.3791, 0.169643);
        of->AddPoint(139.919, 0.589286);
        of->AddPoint(347.907, 0.607143);
        of->AddPoint(1224.16, 0.607143);
        of->AddPoint(3071, 0.616071);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CHEST_CONTRAST_ENHANCED:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(67.0106, 0.54902, 0.25098, 0.14902);
        ctf->AddRGBPoint(251.105, 0.882353, 0.603922, 0.290196);
        ctf->AddRGBPoint(439.291, 1, 0.937033, 0.954531);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(67.0106, 0);
        of->AddPoint(251.105, 0.446429);
        of->AddPoint(439.291, 0.625);
        of->AddPoint(3071, 0.616071);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CHEST_VESSELS:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(-1278.35, 0.54902, 0.25098, 0.14902);
        ctf->AddRGBPoint(22.8277, 0.882353, 0.603922, 0.290196);
        ctf->AddRGBPoint(439.291, 1, 0.937033, 0.954531);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-1278.35, 0);
        of->AddPoint(22.8277, 0.428571);
        of->AddPoint(439.291, 0.625);
        of->AddPoint(3071, 0.616071);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CORONARY_ARTERIES:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(136.47, 0, 0, 0);
        ctf->AddRGBPoint(159.215, 0.159804, 0.159804, 0.159804);
        ctf->AddRGBPoint(318.43, 0.764706, 0.764706, 0.764706);
        ctf->AddRGBPoint(478.693, 1, 1, 1);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(136.47, 0);
        of->AddPoint(159.215, 0.258929);
        of->AddPoint(318.43, 0.571429);
        of->AddPoint(478.693, 0.776786);
        of->AddPoint(3661, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CORONARY_ARTERIES_2:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(142.677, 0, 0, 0);
        ctf->AddRGBPoint(145.016, 0.615686, 0, 0.0156863);
        ctf->AddRGBPoint(192.174, 0.909804, 0.454902, 0);
        ctf->AddRGBPoint(217.24, 0.972549, 0.807843, 0.611765);
        ctf->AddRGBPoint(384.347, 0.909804, 0.909804, 1);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(142.677, 0);
        of->AddPoint(145.016, 0.116071);
        of->AddPoint(192.174, 0.5625);
        of->AddPoint(217.24, 0.776786);
        of->AddPoint(384.347, 0.830357);
        of->AddPoint(3661, 0.830357);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_CORONARY_ARTERIES_3:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(128.643, 0, 0, 0);
        ctf->AddRGBPoint(129.982, 0.615686, 0, 0.0156863);
        ctf->AddRGBPoint(173.636, 0.909804, 0.454902, 0);
        ctf->AddRGBPoint(255.884, 0.886275, 0.886275, 0.886275);
        ctf->AddRGBPoint(584.878, 0.968627, 0.968627, 0.968627);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(128.643, 0);
        of->AddPoint(129.982, 0.0982143);
        of->AddPoint(173.636, 0.669643);
        of->AddPoint(255.884, 0.857143);
        of->AddPoint(584.878, 0.866071);
        of->AddPoint(3661, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_VOLUME_BONE:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(-451, 0, 0, 0);
        ctf->AddRGBPoint(-450, 0.0556356, 0.0556356, 0.0556356);
        ctf->AddRGBPoint(1050, 1, 1, 1);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(-451, 0);
        of->AddPoint(-450, 1);
        of->AddPoint(1050, 1);
        of->AddPoint(3661, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_FAT:
    {
        // color
        ctf->AddRGBPoint(-1000, 0.3, 0.3, 1);
        ctf->AddRGBPoint(-497.5, 0.3, 1, 0.3);
        ctf->AddRGBPoint(-99, 0, 0, 1);
        ctf->AddRGBPoint(-76.946, 0, 1, 0);
        ctf->AddRGBPoint(-65.481, 0.835431, 0.888889, 0.0165387);
        ctf->AddRGBPoint(83.89, 1, 0, 0);
        ctf->AddRGBPoint(463.28, 1, 0, 0);
        ctf->AddRGBPoint(659.15, 1, 0.912535, 0.0374849);
        ctf->AddRGBPoint(2952, 1, 0.300267, 0.299886);

        // scalar opacity
        of->AddPoint(-1000, 0);
        of->AddPoint(-100, 0);
        of->AddPoint(-99, 0.15);
        of->AddPoint(-60, 0.15);
        of->AddPoint(-59, 0);
        of->AddPoint(101.2, 0);
        of->AddPoint(952, 0);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_LIVER_VASCULATURE:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(149.113, 0, 0, 0);
        ctf->AddRGBPoint(157.884, 0.501961, 0.25098, 0);
        ctf->AddRGBPoint(339.96, 0.695386, 0.59603, 0.36886);
        ctf->AddRGBPoint(388.526, 0.854902, 0.85098, 0.827451);
        ctf->AddRGBPoint(1197.95, 1, 1, 1);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(149.113, 0);
        of->AddPoint(157.884, 0.482143);
        of->AddPoint(339.96, 0.660714);
        of->AddPoint(388.526, 0.830357);
        of->AddPoint(1197.95, 0.839286);
        of->AddPoint(3661, 0.848214);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_LUNG:
    {
        // color
        ctf->AddRGBPoint(-1000, 0.3, 0.3, 1);
        ctf->AddRGBPoint(-600, 0, 0, 1);
        ctf->AddRGBPoint(-530, 0.134704, 0.781726, 0.0724558);
        ctf->AddRGBPoint(-460, 0.929244, 1, 0.109473);
        ctf->AddRGBPoint(-400, 0.888889, 0.254949, 0.0240258);
        ctf->AddRGBPoint(2952, 1, 0.3, 0.3);

        // scalar opacity
        of->AddPoint(-1000, 0);
        of->AddPoint(-600, 0);
        of->AddPoint(-599, 0.15);
        of->AddPoint(-400, 0.15);
        of->AddPoint(-399, 0);
        of->AddPoint(2952, 0);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_MIP:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(-637.62, 1, 1, 1);
        ctf->AddRGBPoint(700, 1, 1, 1);
        ctf->AddRGBPoint(3071, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-637.62, 0);
        of->AddPoint(700, 1);
        of->AddPoint(3071, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::MaximumIntensity);

        break;
    }
    case VolumeRenderMaterialPresets::CT_MUSCLE:
    {
        // color
        ctf->AddRGBPoint(-3024, 0, 0, 0);
        ctf->AddRGBPoint(-155.407, 0.54902, 0.25098, 0.14902);
        ctf->AddRGBPoint(217.641, 0.882353, 0.603922, 0.290196);
        ctf->AddRGBPoint(419.736, 1, 0.937033, 0.954531);
        ctf->AddRGBPoint(3071, 0.827451, 0.658824, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-155.407, 0);
        of->AddPoint(217.641, 0.676471);
        of->AddPoint(419.736, 0.833333);
        of->AddPoint(3071, 0.803922);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(0.9);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_PULMONARY_ARTERIES:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(-568.625, 0, 0, 0);
        ctf->AddRGBPoint(-364.081, 0.396078, 0.301961, 0.180392);
        ctf->AddRGBPoint(-244.813, 0.611765, 0.352941, 0.0705882);
        ctf->AddRGBPoint(18.2775, 0.843137, 0.0156863, 0.156863);
        ctf->AddRGBPoint(447.798, 0.752941, 0.752941, 0.752941);
        ctf->AddRGBPoint(3592.73, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(-568.625, 0);
        of->AddPoint(-364.081, 0.0714286);
        of->AddPoint(-244.813, 0.401786);
        of->AddPoint(18.2775, 0.607143);
        of->AddPoint(447.798, 0.830357);
        of->AddPoint(3592.73, 0.839286);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_SOFT_TISSUE:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(-167.01, 0, 0, 0);
        ctf->AddRGBPoint(-160, 0.0556356, 0.0556356, 0.0556356);
        ctf->AddRGBPoint(240, 1, 1, 1);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(-167.01, 0);
        of->AddPoint(-160, 1);
        of->AddPoint(240, 1);
        of->AddPoint(3661, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_AIR:
    {
        // color
        ctf->AddRGBPoint(-3024, 1, 1, 1);
        ctf->AddRGBPoint(-900.0, 0.2, 1.0, 1.0);
        ctf->AddRGBPoint(-500.0, 0.3, 0.3, 1.0);
        ctf->AddRGBPoint(3071, 0, 0, 0);

        // scalar opacity
        of->AddPoint(-3024, 0.705882);
        of->AddPoint(-900.0, 0.715686);
        of->AddPoint(-500.0, 0);
        of->AddPoint(3071, 0);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.9);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::CT_X_RAY:
    {
        // color
        ctf->AddRGBPoint(-3024, 1, 1, 1);
        ctf->AddRGBPoint(3071, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-3024, 0);
        of->AddPoint(-200.0, 0.0);
        of->AddPoint(1500.0, 0.05);
        of->AddPoint(3071, 0.05);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.1);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0.2);
        volProp->SetSpecularPower(10);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::MaximumIntensity);

        break;
    }
    case VolumeRenderMaterialPresets::MR_ANGIO:
    {
        // color
        ctf->AddRGBPoint(-2048, 0, 0, 0);
        ctf->AddRGBPoint(151.354, 0, 0, 0);
        ctf->AddRGBPoint(158.279, 0.74902, 0.376471, 0);
        ctf->AddRGBPoint(190.112, 1, 0.866667, 0.733333);
        ctf->AddRGBPoint(200.873, 0.937255, 0.937255, 0.937255);
        ctf->AddRGBPoint(3661, 1, 1, 1);

        // scalar opacity
        of->AddPoint(-2048, 0);
        of->AddPoint(151.354, 0);
        of->AddPoint(158.279, 0.4375);
        of->AddPoint(190.112, 0.580357);
        of->AddPoint(200.873, 0.732143);
        of->AddPoint(3661, 0.741071);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::MR_DEFAULT:
    {
        // color
        ctf->AddRGBPoint(0, 0, 0, 0);
        ctf->AddRGBPoint(20, 0.168627, 0, 0);
        ctf->AddRGBPoint(40, 0.403922, 0.145098, 0.0784314);
        ctf->AddRGBPoint(120, 0.780392, 0.607843, 0.380392);
        ctf->AddRGBPoint(220, 0.847059, 0.835294, 0.788235);
        ctf->AddRGBPoint(1024, 1, 1, 1);

        // scalar opacity
        of->AddPoint(0, 0);
        of->AddPoint(20, 0);
        of->AddPoint(40, 0.15);
        of->AddPoint(120, 0.3);
        of->AddPoint(220, 0.375);
        of->AddPoint(1024, 0.5);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    case VolumeRenderMaterialPresets::MR_MIP:
    {
        // color
        ctf->AddRGBPoint(0, 1, 1, 1);
        ctf->AddRGBPoint(98.3725, 1, 1, 1);
        ctf->AddRGBPoint(416.637, 1, 1, 1);
        ctf->AddRGBPoint(2800, 1, 1, 1);

        // scalar opacity
        of->AddPoint(0, 0);
        of->AddPoint(98.3725, 0);
        of->AddPoint(416.637, 1);
        of->AddPoint(2800, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.2);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(0);
        volProp->SetSpecular(0);
        volProp->SetSpecularPower(1);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::MaximumIntensity);

        break;
    }
    case VolumeRenderMaterialPresets::MR_T2_BRAIN:
    {
        // color
        ctf->AddRGBPoint(0, 0, 0, 0);
        ctf->AddRGBPoint(98.7223, 0.956863, 0.839216, 0.192157);
        ctf->AddRGBPoint(412.406, 0, 0.592157, 0.807843);
        ctf->AddRGBPoint(641, 1, 1, 1);

        // scalar opacity
        of->AddPoint(0, 0);
        of->AddPoint(36.05, 0);
        of->AddPoint(218.302, 0.171429);
        of->AddPoint(412.406, 1);
        of->AddPoint(641, 1);

        // gradient opacity
        gf->AddPoint(0, 1);
        gf->AddPoint(255, 1);

        // lighting / material properties
        volProp->SetAmbient(0.3);
        volProp->SetDiffuse(1);
        volProp->SetInterpolationTypeToLinear();
        volProp->SetShade(1);
        volProp->SetSpecular(0.5);
        volProp->SetSpecularPower(40);

        // blend mode
        mat->setBlendMode(VolumeRenderMaterial::BlendMode::Alpha);

        break;
    }
    }

    return mat;
}

const char*
VolumeRenderMaterialPresets::getPresetName(int p)
{
    if (p >= VolumeRenderMaterialPresets::NUM_PRESETS)
    {
        return "";
    }
    const char* presets[] = {
        "CT_AAA",
        "CT_AAA2",
        "CT_BONE",
        "CT_BONES",
        "CT_CARDIAC",
        "CT_CARDIAC2",
        "CT_CARDIAC3",
        "CT_CHEST_CONTRAST_ENHANCED",
        "CT_CHEST_VESSELS",
        "CT_CORONARY_ARTERIES",
        "CT_CORONARY_ARTERIES_2",
        "CT_CORONARY_ARTERIES_3",
        "CT_VOLUME_BONE",
        "CT_FAT",
        "CT_LIVER_VASCULATURE",
        "CT_LUNG",
        "CT_MIP",
        "CT_MUSCLE",
        "CT_PULMONARY_ARTERIES",
        "CT_SOFT_TISSUE",
        "CT_AIR",
        "CT_X_RAY",
        "MR_ANGIO",
        "MR_DEFAULT",
        "MR_MIP",
        "MR_T2_BRAIN",
    };
    return presets[p];
}
}
