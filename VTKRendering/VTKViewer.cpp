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
#include <vtkRendererCollection.h>
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
    VTKRenderer(){}

    ///
    /// \brief Callback method executed by the render window interactor.
    ///
    void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId, void *callData)
    {
        switch(eventId)
        {
            case vtkCommand::TimerEvent:
            {
                if(timerId == *static_cast<int*>(callData) &&
                    !this->viewer->isTerminated())
                {

                    if (this->cameraControllerData != nullptr)
                    {
                        vtkRenderer* renderer =
                            this->renderWindow->GetRenderers()->GetFirstRenderer();
                        updateCamera(renderer);
                        renderer->ResetCameraClippingRange();
                    }

                    this->renderWindow->Render();
                }
                break;
            }
            case vtkCommand::ExitEvent:
            {
                // Close the window
                this->renderWindow->Finalize();

                // Stop the interactor
                this->renderWindowInteractor->TerminateApp();
                this->renderWindowInteractor->DestroyTimer(this->timerId);
                this->viewer->terminate();
                break;
            }
        }
    }

	///
	/// \brief Update the camera from external source (optional)
	///
    void updateCamera(vtkRenderer* renderer)
    {
        vtkCamera* camera = renderer->GetActiveCamera();

        camera->SetPosition(
            cameraControllerData->position[0],
            cameraControllerData->position[1],
            cameraControllerData->position[2]);

        camera->SetViewUp(
            cameraControllerData->upVector[0],
            cameraControllerData->upVector[1],
            cameraControllerData->upVector[2]);

        camera->SetFocalPoint(
            cameraControllerData->focus[0],
            cameraControllerData->focus[1],
            cameraControllerData->focus[2]);
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
        this->renderWindow->Render();
        this->renderWindowInteractor->Start();
    }

    ///
    /// \brief Remove renderer from render window.
    ///
    void removeRenderer(vtkRenderer* renderer)
    {
        this->renderWindow->RemoveRenderer(renderer);
    }

    ///
    /// \brief Add light to vtk renderer
    ///
    void addLight(vtkRenderer *renderer, Light *light)
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

    ///
    /// \brief Add camera to vtk renderer
    ///
    void addCamera(vtkRenderer *renderer, Camera *camera)
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
            // Set up lights
            auto lights = ro.scene->getLights();
            for(auto &light : lights)
            {
                this->addLight(renderer.GetPointer(),light.get());
            }

            // Set up camera
            auto camera = ro.scene->getCamera()->getDefaultCamera();
            if(camera)
            {
                this->addCamera(renderer.GetPointer(),camera.get());
            }

            // Set up actors
            for(const auto &object : ro.scene->getSceneObject())
            {
                delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(
                    object->getRenderDelegate());
                if (delegate)
                {
                    renderer->AddActor(delegate->getActor());
                }
            }
        }

        // Add actors from objects directly attached to the viewer.
        for(const auto &object : this->viewer->objectList)
        {
            delegate = std::dynamic_pointer_cast<VTKRenderDelegate>(
                object->getRenderDelegate());
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
        this->renderWindowInteractor->AddObserver(vtkCommand::TimerEvent,this);
        this->renderWindowInteractor->AddObserver(vtkCommand::ExitEvent, this);

        if (viewer->viewerRenderDetail & SIMMEDTK_DISABLE_MOUSE_INTERACTION)
        {
            renderWindowInteractor->RemoveObservers("LeftButtonPressEvent");
            renderWindowInteractor->RemoveObservers("RightButtonPressEvent");
            renderWindowInteractor->RemoveObservers("MiddleButtonPressEvent");

            renderWindowInteractor->RemoveObservers("LeftButtonReleaseEvent");
            renderWindowInteractor->RemoveObservers("RightButtonReleaseEvent");
            renderWindowInteractor->RemoveObservers("MiddleButtonReleaseEvent");

            renderWindowInteractor->RemoveObservers("MouseWheelBackwardEvent");
            renderWindowInteractor->RemoveObservers("MouseWheelForwardEvent");
            renderWindowInteractor->RemoveObservers("MouseMoveEvent");
        }

        // Initialize must be called prior to creating timer events.
        this->renderWindowInteractor->Initialize();
        this->timerId = renderWindowInteractor->CreateRepeatingTimer(1000.0/60.0);

        if ( viewer->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_GLOBALAXIS)
        {
            vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();

            renderer->AddActor(axes);
        }

        //if(0)// viewer->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_GLOBAL_AXIS)
        //{
        //    // FIXME: This causes the renderer to crash.
        //    vtkNew<vtkAxesActor> axesActor;
        //    vtkNew<vtkOrientationMarkerWidget> orientationWidget;
        //    orientationWidget->SetOrientationMarker( axesActor.GetPointer() );
        //    orientationWidget->SetInteractor( renderWindowInteractor.GetPointer() );
        //    orientationWidget->SetEnabled( 1 );
        //    orientationWidget->InteractiveOff();
        //}

        // Set up background
        if (viewer->viewerRenderDetail & SIMMEDTK_VIEWERRENDER_FADEBACKGROUND)
        {
            auto bgTop = this->viewer->getRenderDetail()->getBackgroundTop().getValue();
            auto bgBottom = this->viewer->getRenderDetail()->getBackgroundBottom().getValue();

            renderer->GradientBackgroundOn();
            renderer->SetBackground(bgBottom[0], bgBottom[1], bgBottom[2]);
            renderer->SetBackground2(bgTop[0], bgTop[1], bgTop[2]);
        }
        else
        {
            auto background = this->viewer->getRenderDetail()->getBackground().getValue();
            renderer->SetBackground(background[0], background[1], background[2]);
        }
    }

public:
    int timerId;
    VTKViewer *viewer;
    vtkNew<vtkRenderWindow> renderWindow;
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    std::shared_ptr<cameraConfigurationData> cameraControllerData;
};

VTKViewer::VTKViewer() : renderer(Core::make_unique<VTKRenderer> (this))
{
    this->name = "VTKViewer";
}

VTKViewer::~VTKViewer()
{
}

void VTKViewer::exec()
{
    this->render();
    this->terminationCompleted = true;
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

vtkRenderWindow* VTKViewer::getRenderWindow() const
{
    return this->renderer->getRenderWindow();
}

vtkCamera* VTKViewer::getVtkCamera()
{
    return this->renderer->getRenderWindow()->GetRenderers()->
        GetFirstRenderer()->GetActiveCamera();
}

void VTKViewer::setCameraControllerData(std::shared_ptr<cameraConfigurationData> camData)
{
    renderer->cameraControllerData = camData;
}

vtkRenderer* VTKViewer::getVtkRenderer()
{
    return this->renderer->getRenderWindow()->GetRenderers()->GetFirstRenderer();
}

vtkRenderWindowInteractor* VTKViewer::getVtkRenderWindowInteractor()
{
    return this->renderer->renderWindowInteractor.GetPointer();
}

void VTKViewer::addChartActor(vtkContextActor* chartActor, vtkContextScene* chartScene)
{
    vtkRenderer* rendererVtk =
        this->renderer->getRenderWindow()->GetRenderers()->GetFirstRenderer();

    rendererVtk->AddActor(chartActor);

    chartScene->SetRenderer(rendererVtk);
}

void VTKViewer::initRenderingContext()
{
    this->renderer->addRenderer();
}

RegisterFactoryClass(ViewerBase,
                     VTKViewer,
                     RenderDelegate::RendererType::VTK)
