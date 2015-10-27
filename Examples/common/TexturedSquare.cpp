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

#include "TexturedSquare.h"

#include "Core/Factory.h"
#include "Core/StaticSceneObject.h"
#include "Core/RenderDelegate.h"
#include "Rendering/TextureManager.h"

namespace mstk {
namespace Examples {
namespace Common {

TexturedSquare::TexturedSquare(bool vtkRenderer)
    : useVTK(vtkRenderer)
{
}

void TexturedSquare::useVTKRenderer(bool b)
{
    this->useVTK = b;
}

std::shared_ptr<StaticSceneObject> TexturedSquare::getStaticSceneObject()
{
    return this->square;
}

void TexturedSquare::setup()
{
    squareModel = std::make_shared<MeshModel>();
    squareModel->load("models/square.obj");

    renderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_FACES | SIMMEDTK_RENDER_TEXTURE);
    squareModel->setRenderDetail(renderDetail);

    square = std::make_shared<StaticSceneObject>();
    square->setModel(squareModel);

    // If you want to use the GL renderer you need to specify the appropiate render delegates
    // This can be automated in the future, for now VTK is the default renderer and the delegates
    // need to be reset.
    if(!useVTK)
    {
        auto renderDelegate = Factory<RenderDelegate>::createConcreteClassForGroup(
            "StaticSceneObjectRenderDelegate", RenderDelegate::RendererType::Other);
        square->setRenderDelegate(renderDelegate);

        renderDelegate = Factory<RenderDelegate>::createConcreteClassForGroup(
            "MeshRenderDelegate", RenderDelegate::RendererType::Other);
        squareModel->getMesh()->setRenderDelegate(renderDelegate);
    }
}

void TexturedSquare::setTexture(const std::string &t)
{
    renderDetail->setTextureFilename(t);

    if(!useVTK)
    {
        std::static_pointer_cast<SurfaceMesh>(squareModel->getMesh())->assignTexture(t);
    }
}

} //Common
} //Examples
} //mstk
