if(NOT WIN32)
  message(WARNING "System not supported. Only Windows is supported for External_Libusb.cmake.")
  message(WARNING "Libusb-1.0 should already be installed on your system.")
  return()
endif()

#-----------------------------------------------------------------------------
# Set project prefix path
#-----------------------------------------------------------------------------
set(Libusb_PREFIX ${CMAKE_BINARY_DIR}/External/Libusb)
set(Libusb_EXTRACT_DIR ${Libusb_PREFIX}/libusb-1.0.20)

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
set(libusb_libdir "MS32")
set(libusb_exedir "bin32")
if(${CMAKE_GENERATOR} MATCHES "Win64")
  set(libusb_libdir "MS64")
  set(libusb_exedir "bin64")
endif()

set(copy_libusb_lib_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_EXTRACT_DIR}/${libusb_libdir}/dll/libusb-1.0.lib
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/libusb-1.0.lib
  )

set(copy_libusb_dll_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_EXTRACT_DIR}/${libusb_libdir}/dll/libusb-1.0.dll
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/libusb-1.0.dll
  )

set(copy_libusb_pdb_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_EXTRACT_DIR}/${libusb_libdir}/dll/libusb-1.0.pdb
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/libusb-1.0.pdb
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Libusb
  URL http://downloads.sourceforge.net/project/libusb/libusb-1.0/libusb-1.0.21/libusb-1.0.21.7z
  URL_MD5 7fbcf5580b8ffc88f3af6eddd638de9f
  DOWNLOAD_DIR ${Libusb_PREFIX}
  SOURCE_DIR ${Libusb_EXTRACT_DIR}
  BINARY_DIR ${Libusb_EXTRACT_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    ${copy_libusb_lib_command}
    COMMAND ${copy_libusb_dll_command}
    COMMAND ${copy_libusb_pdb_command}
  RELATIVE_INCLUDE_PATH "include/libusb-1.0"
  #VERBOSE
  )
