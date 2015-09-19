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
#include "Core/Factory.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkLight.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

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
        // The actors are obtained from VTKRenderDelegates
        std::shared_ptr<VTKRenderDelegate> delegate;
        for(auto &ro : this->viewer->renderOperations)
        {

            // Set up lighting
            auto lights = ro.scene->getLights();
            for(auto &light : lights)
            {
                auto position = light->lightPos.getPosition();
                auto colorDiffuse = light->lightColorDiffuse.getValue();
                auto colorAmbient = light->lightColorDiffuse.getValue();
                auto colorSpecular = light->lightColorDiffuse.getValue();
                auto coneAngle = light->spotCutOffAngle;
                auto focalPoint = light->focusPosition;
                auto constAttenuation = light->attn_constant;
                auto linearAttenuation = light->attn_linear;
                auto quadAttenuation = light->attn_quadratic;

                vtkNew<vtkLight> l;
                l->SetLightTypeToSceneLight();
                l->SetPosition(position[0],position[1],position[2]);
                l->SetDiffuseColor(colorDiffuse[0],colorDiffuse[1],colorDiffuse[2]);
                l->SetAmbientColor(colorAmbient[0],colorAmbient[1],colorAmbient[2]);
                l->SetSpecularColor(colorSpecular[0],colorSpecular[1],colorSpecular[2]);
                l->SetConeAngle(coneAngle);
                l->SetFocalPoint(focalPoint[0],focalPoint[1],focalPoint[2]);
                l->SetAttenuationValues(constAttenuation,linearAttenuation,quadAttenuation);
                renderer->AddLight(l.GetPointer());
            }

            // Set up camera
            auto camera = ro.scene->getCamera();
            if(camera)
            {
                auto position = camera->getPos();
                auto focus = camera->getFocus();
                auto upView = camera->getOrientation() * core::Vec3f::UnitZ();
                auto viewangle = camera->getViewAngleDeg();
                auto nearClippingRange = camera->getNearClipDist();
                auto farClippingRange = camera->getFarClipDist();
                auto zoom = camera->getZoom();

                vtkNew<vtkCamera> c;
                c->SetPosition(position[0],position[1],position[2]);
                c->SetFocalPoint(focus[0],focus[1],focus[2]);
                c->SetViewAngle(viewangle);
                c->SetClippingRange(nearClippingRange,farClippingRange);
                c->Zoom(zoom);
                c->SetViewUp(upView[0],upView[1],upView[2]);

                renderer->SetActiveCamera(c.GetPointer());
                renderer->ResetCamera();
            }

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

        if(viewer->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS)
        {
            std::cerr << "Axis display does not work properly." << std::endl;
//             vtkNew<vtkAxesActor> axesActor;
//
//             vtkNew<vtkOrientationMarkerWidget> orientationWidget;
//             orientationWidget->SetOrientationMarker( axesActor.GetPointer() );
//             orientationWidget->SetInteractor( renderWindowInteractor.GetPointer() );
//             orientationWidget->SetEnabled( 1 );
//             orientationWidget->InteractiveOff();
        }

        // Set up background
        auto background = this->viewer->getRenderDetail()->getBackground().getValue();
        renderer->SetBackground(background[0],background[1],background[2]);
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
