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

#include "VtkRendering/VtkViewer.h"
#include "VtkRendering/VtkRenderDelegate.h"
#include "Core/MakeUnique.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

///
/// \brief Wrapper to the vtkRendering pipeline
///
class VtkViewer::VtkRenderer
{
public:
    VtkRenderer(VtkViewer *activeViewer) :
        viewer(activeViewer),
        renderWindow(vtkRenderWindow::New()),
        renderWindowInteractor(vtkRenderWindowInteractor::New())
    {
    }

    ///
    /// \brief Return the render window
    ///
    vtkSmartPointer<vtkRenderWindow> get() const
    {
        return this->renderWindow;
    }

    ///
    /// \brief Render scene
    ///
    void render()
    {
        this->viewer->adjustFPS();
        this->renderWindow->Render();
        this->renderWindowInteractor->Start();
    }

    void removeRenderer(vtkRenderer* renderer)
    {
        this->renderWindow->RemoveRenderer(renderer);
    }

    ///
    /// \brief Add renderer to the render windows
    ///
    void addRenderer()
    {
        // Create a new renderer and add actors to it.
        vtkNew<vtkRenderer> renderer;
        if (viewer->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
        {
            this->renderWindow->FullScreenOn();
        }

        this->renderWindowInteractor->SetRenderWindow(renderWindow);

        // The actors are obtained from VtkRenderDelegates
        std::shared_ptr<VtkRenderDelegate> delegate;
        for(auto &ro : this->viewer->renderOperations)
        {
            for(const auto &object : ro.scene->getSceneObject())
            {
                delegate = std::dynamic_pointer_cast<VtkRenderDelegate>(object->getRenderDelegate());
                if (delegate)
                {
                    renderer->AddActor(delegate->getActor());
                }
            }
        }
        for(const auto &object : this->viewer->objectList)
        {
            delegate = std::dynamic_pointer_cast<VtkRenderDelegate>(object->getRenderDelegate());
            if (delegate)
            {
                renderer->AddActor(delegate->getActor());
            }
        }

        if(renderer->GetActors()->GetNumberOfItems() > 0)
        {
            this->renderWindow->AddRenderer(renderer.GetPointer());
        }
    }

public:
    VtkViewer *viewer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
};

VtkViewer::VtkViewer() : renderer(make_unique<VtkRenderer> (this))
{
}

VtkViewer::~VtkViewer()
{
}

void VtkViewer::exec()
{
    this->init();
    this->render();
}
void VtkViewer::render()
{
    if(this->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    this->beginModule();

    this->renderer->render();

    this->endModule();
}
bool VtkViewer::isValid()
{
    return ( this->renderer != nullptr )
           && ( this->renderer->renderWindow != nullptr )
           && ( this->renderer->renderWindowInteractor != nullptr );
}
void VtkViewer::initResources()
{
    this->renderer->addRenderer();
}

