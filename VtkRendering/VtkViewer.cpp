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
#include <vtkInteractorStyleSwitch.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>

///
/// \brief Wrapper to the vtkRendering pipeline
///
class VtkViewer::VtkRenderer : public vtkCommand
{
    //typedef void(*FunctionType)(vtkObject *, unsigned long, void*, void*);
    //using FunctionType = void(vtkObject *,unsigned long,void*,void*);
    //using CallBackFunctionType = std::function<FunctionType>;

public:
    VtkRenderer(VtkViewer *activeViewer) :
        timerId(-1),
        viewer(activeViewer)
    {
        auto timerEvent = [&](vtkObject*,unsigned long,void *callData, void*)
                          {
                              if(this->timerId == *static_cast<int*>(callData))
                              {
                                  this->renderWindow->Render();
                              }
                          };
        auto keyPress =  [](vtkObject *caller,unsigned long,void*,void*)
                         {
                             vtkRenderWindowInteractor *iren =
                                static_cast<vtkRenderWindowInteractor*>(caller);
                             // Close the window
                             iren->GetRenderWindow()->Finalize();

                             // Stop the interactor
                             iren->TerminateApp();
                             std::cout << "Closing window..." << std::endl;
                         };

        //callBacks.emplace(vtkCommand::TimerEvent,timerEvent);
        //callBacks.emplace(vtkCommand::KeyPressEvent,keyPress);
    }

    ///
    /// \brief Callback method
    ///
    void Execute(vtkObject *vtkNotUsed(caller),
                         unsigned long vtkNotUsed(eventId),
                         void *callData)
    {
        if(timerId == *static_cast<int*>(callData))
        {
            this->renderWindow->Render();
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
        this->timerId = renderWindowInteractor->CreateRepeatingTimer(1000.0/60.0);
        this->renderWindowInteractor->Start();
        this->renderWindowInteractor->DestroyTimer(this->timerId);
    }

    void setObservers()
    {
//        for(auto f : this->callBacks)
//        {
//            vtkNew<vtkCallbackCommand> callback;
//            callback->SetCallback(f.second.target<FunctionType>());
//            this->renderWindowInteractor->AddObserver(f.first,callback.GetPointer());
//        }
    }

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

        this->renderWindow->SetWindowName("MSTK");
        this->renderWindowInteractor->SetRenderWindow(renderWindow.GetPointer());
        vtkSmartPointer<vtkInteractorStyleSwitch> style =
        vtkSmartPointer<vtkInteractorStyleSwitch>::New();
        style->SetCurrentStyleToTrackballCamera();
        renderWindowInteractor->SetInteractorStyle( style );
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
    int timerId;
    VtkViewer *viewer;
    vtkNew<vtkRenderWindow> renderWindow;
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
//    std::map<vtkCommand::EventIds,CallBackFunctionType> callBacks;
};

VtkViewer::VtkViewer() : renderer(Core::make_unique<VtkRenderer> (this))
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
           && ( this->renderer->renderWindow.GetPointer() != nullptr )
           && ( this->renderer->renderWindowInteractor.GetPointer() != nullptr );
}
void VtkViewer::initResources()
{
    this->renderer->addRenderer();
}

