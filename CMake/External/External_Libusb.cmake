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
if(${CMAKE_GENERATOR} MATCHES "Win64")
  set(libusb_libdir "MS64")
endif()

set(copy_libusb_headers_command
  ${CMAKE_COMMAND} -E copy_directory
  ${Libusb_EXTRACT_DIR}/include
  ${CMAKE_INSTALL_PREFIX}/include
  )
set(copy_libusb_lib_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_EXTRACT_DIR}/${libusb_libdir}/static/libusb-1.0.lib
  ${CMAKE_INSTALL_PREFIX}/lib/libusb-1.0.lib
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Libusb
  URL http://downloads.sourceforge.net/project/libusb/libusb-1.0/libusb-1.0.21/libusb-1.0.21.7z https://data.kitware.com/api/v1/item/59cbcefd8d777f7d33e9d9d7/download
  URL_MD5 7fbcf5580b8ffc88f3af6eddd638de9f
  DOWNLOAD_DIR ${Libusb_PREFIX}
  SOURCE_DIR ${Libusb_EXTRACT_DIR}
  BINARY_DIR ${Libusb_EXTRACT_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${copy_libusb_headers_command}
    COMMAND ${copy_libusb_lib_command}
  RELATIVE_INCLUDE_PATH "include/libusb-1.0"
  #VERBOSE
  )
