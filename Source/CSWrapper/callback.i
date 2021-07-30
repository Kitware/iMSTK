%callback("%s_cb");
/* std::string imstk::KeyboardDeviceClient::keyPress(); */
std::string KeyboardDeviceClient_getKeyPress();
std::string Module_getPostUpdate();
std::string Module_getPreUpdate();
std::string SceneManager_getPreUpdate();
std::string SceneManager_getPostUpdate();
%nocallback;

%{
std::string KeyboardDeviceClient_getKeyPress() {
  return imstk::KeyboardDeviceClient::keyPress();
}

std::string Module_getPostUpdate() {
  return imstk::Module::postUpdate();
}
std::string Module_getPreUpdate() {
  return imstk::Module::preUpdate();
}
std::string SceneManager_getPostUpdate() {
  return imstk::SceneManager::postUpdate();
}
std::string SceneManager_getPreUpdate() {
  return imstk::SceneManager::preUpdate();
}
%}

