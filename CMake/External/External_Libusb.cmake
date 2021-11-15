if(NOT WIN32)
  message(WARNING "System not supported. Only Windows is supported for External_Libusb.cmake.")
  message(WARNING "Libusb-1.0 should already be installed on your system.")
  return()
endif()

include(imstkAddExternalProject)

# Set Libusb_SOURCE_DIR and Libusb_PREFIX
imstk_define_external_dirs( Libusb )

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
set(Libusb_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

set(libusb_libdir "MS32")
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(libusb_libdir "MS64")
endif()

set(copy_libusb_headers_command
  ${CMAKE_COMMAND} -E copy_directory
  ${Libusb_SOURCE_DIR}/include
  ${Libusb_INSTALL_DIR}/include
  )
set(copy_libusb_lib_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_SOURCE_DIR}/${libusb_libdir}/dll/libusb-1.0.lib
  ${Libusb_INSTALL_DIR}/lib/libusb-1.0.lib
  )
set(copy_libusb_dll_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_SOURCE_DIR}/${libusb_libdir}/dll/libusb-1.0.dll
  ${Libusb_INSTALL_DIR}/bin
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
imstk_add_external_project( Libusb
  URL http://downloads.sourceforge.net/project/libusb/libusb-1.0/libusb-1.0.21/libusb-1.0.21.7z https://data.kitware.com/api/v1/item/59cbcefd8d777f7d33e9d9d7/download
  URL_MD5 7fbcf5580b8ffc88f3af6eddd638de9f
  DOWNLOAD_DIR ${Libusb_PREFIX}
  SOURCE_DIR ${Libusb_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${copy_libusb_headers_command}
    COMMAND ${copy_libusb_lib_command}
    COMMAND ${copy_libusb_dll_command}
  RELATIVE_INCLUDE_PATH "include/libusb-1.0"
  #VERBOSE
)
