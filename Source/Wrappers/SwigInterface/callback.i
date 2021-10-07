%callback("%s_cb");
/* std::string imstk::KeyboardDeviceClient::keyPress(); */
std::string KeyboardDeviceClient_getKeyPress();
std::string Module_getPostUpdate();
std::string Module_getPreUpdate();
std::string SceneManager_getPreUpdate();
std::string SceneManager_getPostUpdate();
std::string VTKViewer_getPreUpdate();
std::string VTKViewer_getPostUpdate();
%nocallback;

%{
    // The following functions in imstk provide pointer functions
    // to acquire a string. Here we specify code to wrap that just
    // directly returns the string
    
    std::string KeyboardDeviceClient_getKeyPress()
    {
        return imstk::KeyboardDeviceClient::keyPress();
    }

    std::string Module_getPostUpdate()
    {
        return imstk::Module::postUpdate();
    }
    std::string Module_getPreUpdate()
    {
        return imstk::Module::preUpdate();
    }
    std::string SceneManager_getPostUpdate()
    {
        return imstk::SceneManager::postUpdate();
    }
    std::string SceneManager_getPreUpdate()
    {
        return imstk::SceneManager::preUpdate();
    }
    std::string VTKViewer_getPostUpdate()
    {
        return imstk::VTKViewer::postUpdate();
    }
    std::string VTKViewer_getPreUpdate()
    {
        return imstk::VTKViewer::preUpdate();
    }
%}