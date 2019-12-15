if(NOT WIN32)
  message(WARNING "System not supported. Only Windows is supported for External_FTD2XX.cmake.")
  message(WARNING "FTD2XX should not be needed on your system.")
  return()
endif()

#-----------------------------------------------------------------------------
# Set project prefix path
#-----------------------------------------------------------------------------
set(FTD2XX_PREFIX ${CMAKE_BINARY_DIR}/External/FTD2XX)
set(FTD2XX_EXTRACT_DIR ${FTD2XX_PREFIX}/ftd2xx-2.12.18)

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
set(ftd2xx_libdir "i386")
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(ftd2xx_libdir "amd64")
endif()

set(copy_ftd2xx_headers_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_EXTRACT_DIR}/ftd2xx.h
  ${CMAKE_INSTALL_PREFIX}/include/ftd2xx/ftd2xx.h
  )
set(copy_ftd2xx_lib_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_EXTRACT_DIR}/${ftd2xx_libdir}/ftd2xx.lib
  ${CMAKE_INSTALL_PREFIX}/lib/ftd2xx.lib
  )
set(copy_ftd2xx_dll_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_EXTRACT_DIR}/${ftd2xx_libdir}/ftd2xx64.dll
  ${CMAKE_INSTALL_PREFIX}/bin/ftd2xx64.dll
  )

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( FTD2XX
  URL http://www.ftdichip.com/Drivers/CDM/CDM%20v2.12.18%20WHQL%20Certified.zip
  URL_MD5 e1a194765d7bcc58968d523484b74140
  DOWNLOAD_DIR ${FTD2XX_PREFIX}
  SOURCE_DIR ${FTD2XX_EXTRACT_DIR}
  BINARY_DIR ${FTD2XX_EXTRACT_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${copy_ftd2xx_headers_command}
    COMMAND ${copy_ftd2xx_lib_command}
    COMMAND ${copy_ftd2xx_dll_command}
  RELATIVE_INCLUDE_PATH "/"
  #VERBOSE
  )
