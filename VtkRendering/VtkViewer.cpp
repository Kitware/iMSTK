/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#include "VtkRendering/VtkViewer.h"
#include "VtkRendering/VtkRenderDelegate.h"

///
/// \brief Wrapper to the vtkRendering pipeline
///
class VtkViewer::VtkRenderer
{
public:
    VtkRenderer(std::shared_ptr<VtkViewer> activeViewer) :
        viewer(activeViewer),
        renderWindow(vtkRenderWindow::New()),
        renderWindowInteractor(vtkRenderWindowInteractor::New())
    {
    }

    vtkSmartPointer<vtkRenderWindow> get() const
    {
        return this->renderWindow;
    }

    void render()
    {
        this->viewer->adjustFPS();
        this->renderWindow->Render();
        this->renderWindowInteractor->Start();
    }

    void initRenderingContext()
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
                delegate = std::static_pointer_cast<VtkRenderDelegate>(object->getRenderDelegate());
                if (delegate)
                {
                    renderer->AddActor(delegate->getActor());
                }
            }
        }
        if(renderer->GetActors()->GetNumberOfItems() > 0)
        {
            this->renderWindow->AddRenderer(renderer.GetPointer());
        }
    }

private:
    std::shared_ptr<VtkViewer> viewer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
};

VtkViewer::VtkViewer(const VtkViewer& /*other*/)
{
}

VtkViewer::~VtkViewer()
{
}

void VtkViewer::initRenderingContext()
{
    this->init();
    this->renderer->initRenderingContext();
}
void VtkViewer::exec()
{
    this->render();
}
void VtkViewer::render()
{
    if(viewerRenderDetail & SIMMEDTK_VIEWERRENDER_DISABLE)
    {
        return;
    }

    beginModule();

    this->renderer->render();

    endModule();
}
