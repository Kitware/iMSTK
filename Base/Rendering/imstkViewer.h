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

#ifndef imstkViewer_h
#define imstkViewer_h

#include <memory>
#include <unordered_map>

#include "imstkScene.h"
#include "imstkRenderer.h"

#include "vtkSmartPointer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

namespace imstk {
class Viewer
{
public:

    Viewer(std::string name = "iMSTK Viewer")
    {
        m_vtkInteractor->SetRenderWindow( m_vtkRenderWindow );
        m_vtkRenderWindow->SetWindowName(name.data());
        m_vtkRenderWindow->SetSize(1000,800);
    }

    ~Viewer() = default;

    std::shared_ptr<Scene> getCurrentScene() const;
    void setCurrentScene(std::shared_ptr<Scene>scene);
    void setRenderingMode(Renderer::Mode mode);
    void startRenderingLoop();
    void endRenderingLoop();

    vtkSmartPointer<vtkRenderWindow>getVtkRenderWindow() const;


protected:

    vtkSmartPointer<vtkRenderWindow> m_vtkRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> m_vtkInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    std::shared_ptr<Scene> m_currentScene;
    std::unordered_map<std::shared_ptr<Scene>, std::shared_ptr<Renderer>> m_rendererMap;
};
}

#endif // ifndef imstkViewer_h
