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

#pragma once

//===========================================================
// now we define the C++ class

#include "vtkRenderWindowInteractor.h"
#include <X11/Xlib.h>             // Needed for X types in the public interface

class vtkCallbackCommand;
class vtkXRenderWindowInteractor2Internals;

///
/// \class vtkXRenderWindowInteractor2
///
/// \brief This class exists to fix bugs in VTK 9.0, which are, as typing this
/// already merged into VTK master, this class should be deleted when upgrading
/// to yet to be released VTK 10
///
class vtkXRenderWindowInteractor2 : public vtkRenderWindowInteractor
{
public:
    static vtkXRenderWindowInteractor2* New();
    vtkTypeMacro(vtkXRenderWindowInteractor2, vtkRenderWindowInteractor);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    /**
     * Initializes the event handlers without an XtAppContext.  This is
     * good for when you don't have a user interface, but you still
     * want to have mouse interaction.
     */
    void Initialize() override;

    /**
     * Break the event loop on 'q','e' keypress. Want more ???
     */
    void TerminateApp() override;

    /**
     * Run the event loop and return. This is provided so that you can
     * implement your own event loop but yet use the vtk event handling as
     * well.
     */
    void ProcessEvents() override;

    //@{
    /**
     * Enable/Disable interactions.  By default interactors are enabled when
     * initialized.  Initialize() must be called prior to enabling/disabling
     * interaction. These methods are used when a window/widget is being
     * shared by multiple renderers and interactors.  This allows a "modal"
     * display where one interactor is active when its data is to be displayed
     * and all other interactors associated with the widget are disabled
     * when their data is not displayed.
     */
    void Enable() override;
    void Disable() override;
    //@}

    /**
     * Update the Size data member and set the associated RenderWindow's
     * size.
     */
    void UpdateSize(int, int) override;

    /**
     * Re-defines virtual function to get mouse position by querying X-server.
     */
    void GetMousePosition(int* x, int* y) override;

    void DispatchEvent(XEvent*);

protected:
    vtkXRenderWindowInteractor2();
    ~vtkXRenderWindowInteractor2() override;

    /**
     * Update the Size data member and set the associated RenderWindow's
     * size but do not resize the XWindow.
     */
    void UpdateSizeNoXResize(int, int);

    // Using static here to avoid destroying context when many apps are open:
    static int NumAppInitialized;

    Display* DisplayId;
    Window   WindowId;
    Atom     KillAtom;
    int      PositionBeforeStereo[2];
    vtkXRenderWindowInteractor2Internals* Internal;

    // Drag and drop related
    Window XdndSource;
    Atom   XdndPositionAtom;
    Atom   XdndDropAtom;
    Atom   XdndActionCopyAtom;
    Atom   XdndStatusAtom;
    Atom   XdndFinishedAtom;

    //@{
    /**
     * X-specific internal timer methods. See the superclass for detailed
     * documentation.
     */
    int InternalCreateTimer(int timerId, int timerType, unsigned long duration) override;
    int InternalDestroyTimer(int platformTimerId) override;
    //@}

    void FireTimers();

    /**
     * This will start up the X event loop and never return. If you
     * call this method it will loop processing X events until the
     * application is exited.
     */
    void StartEventLoop() override;

private:
    vtkXRenderWindowInteractor2(const vtkXRenderWindowInteractor2&) = delete;
    void operator=(const vtkXRenderWindowInteractor2&) = delete;
};