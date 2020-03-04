message(STATUS "Patching VTK")

file(COPY  ${VTK_SOURCE_DIR}/Common/Core/vtkEventData.h
     DESTINATION ${CMAKE_INSTALL_PREFIX}/include/vtk-8.2/)
file(COPY  ${VTK_SOURCE_DIR}/Rendering/OpenVR/vtkOpenVROverlayInternal.h
     DESTINATION ${CMAKE_INSTALL_PREFIX}/include/vtk-8.2/)