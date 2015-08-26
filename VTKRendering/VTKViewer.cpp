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

#include "VTKRendering/VTKViewer.h"
#include "VTKRendering/VTKRenderDelegate.h"
#include "Core/MakeUnique.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>

///
/// \brief Wrapper to the vtkRendering pipeline
///
class VTKViewer::VTKRenderer : public vtkCommand
{

public:
    VTKRenderer(VTKViewer *activeViewer) :
        timerId(-1),
        viewer(activeViewer)
    {

    }

    ///
    /// \brief Callback method
    ///
    void Execute(vtkObject *caller, unsigned long eventId, void *callData)
    {
        switch(eventId)
        {
            case vtkCommand::TimerEvent:
            {
                if(timerId == *static_cast<int*>(callData))
                {
                    this->renderWindow->Render();
                }
                break;
            }
            case vtkCommand::ExitEvent:
            {
                std::cout << "Closing window..." << std::endl;
                vtkRenderWindowInteractor *iren =
                static_cast<vtkRenderWindowInteractor*>(caller);
                // Close the window
                iren->GetRenderWindow()->Finalize();

                // Stop the interactor
                iren->TerminateApp();
                viewer->setIsValid(false);
                viewer->cleanUp();
                break;
            }
        }
    }

    ///
    /// \brief Return the render window
    ///
    vtkRenderWindow *getRenderWindow() const
    {
        return this->renderWindow.GetPointer();
    }

    ///
    /// \brief Render scene
    ///
    void render()
    {
        this->viewer->adjustFPS();
        this->renderWindow->Render();
        this->renderWindowInteractor->AddObserver(vtkCommand::TimerEvent,this);

        this->renderWindowInteractor->AddObserver(vtkCommand::ExitEvent,this);

        this->timerId = renderWindowInteractor->CreateRepeatingTimer(1000.0/60.0);
        this->renderWindowInteractor->Start();
        this->renderWindowInteractor->DestroyTimer(this->timerId);
    }

    ///
    /// \brief Remove renderer from render window.
    ///
    void removeRenderer(vtkRenderer* renderer)
    {
        this->renderWindow->RemoveRenderer(renderer);
    }

    ///
    /// \brief Add renderer to the render window
    ///
    void addRenderer()
    {
        // Create a new renderer and add actors to it.
        vtkNew<vtkRenderer> renderer;
        if (viewer->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FULLSCREEN)
        {
            this->renderWindow->FullScreenOn();
        }
        else
        {
            this->renderWindow->SetSize(this->viewer->height(),this->viewer->width());
        }

        this->renderWindow->SetWindowName(this->viewer->windowTitle.data());
        this->renderWindowInteractor->SetRenderWindow(this->renderWindow.GetPointer());
        vtkNew<vtkInteractorStyleSwitch> style;
        style->SetCurrentStyleToTrackballCamera();
        renderWindowInteractor->SetInteractorStyle(style.GetPointer());

        // The actors are obtained from VTKRenderDelegates
        std::shared_ptr<VTKRenderDelegate> delegate;
        for(auto &ro : this->viewer->renderOperations)
        {
            for(const auto &object : ro.scene->getSceneObject())
            {
                delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(object->getRenderDelegate());
                if (delegate)
                {
                    renderer->AddActor(delegate->getActor());
                }
            }
        }
        for(const auto &object : this->viewer->objectList)
        {
            delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(object->getRenderDelegate());
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
    int timerId;
    VTKViewer *viewer;
    vtkNew<vtkRenderWindow> renderWindow;
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
};

VTKViewer::VTKViewer() : renderer(Core::make_unique<VTKRenderer> (this))
{
}

VTKViewer::~VTKViewer()
{
}

void VTKViewer::exec()
{
    this->init();
    this->render();
}
void VTKViewer::render()
{
    if(this->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    this->beginModule();

    this->renderer->render();

    this->endModule();
}
void VTKViewer::initResources()
{
    this->renderer->addRenderer();
}

RegisterFactoryClass(ViewerBase,
                     VTKViewer,
                     RenderDelegate::RendererType::VTK)
