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
if(${CMAKE_GENERATOR} MATCHES "Win64")
  set(ftd2xx_libdir "amd64")
endif()

set(copy_ftd2xx_shared_command
  ${CMAKE_COMMAND} -E copy_directory
  ${FTD2XX_EXTRACT_DIR}/${ftd2xx_libdir}
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  )
set(copy_ftd2xx_static_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_EXTRACT_DIR}/${ftd2xx_libdir}/ftd2xx.lib
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/ftd2xx.lib
  )
set(remove_ftd2xx_static_command
  ${CMAKE_COMMAND} -E remove
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/ftd2xx.lib
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
    ${copy_ftd2xx_shared_command}
    COMMAND ${copy_ftd2xx_static_command}
    COMMAND ${remove_ftd2xx_static_command}
  RELATIVE_INCLUDE_PATH "/"
  #VERBOSE
  )
