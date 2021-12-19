if(NOT WIN32)
  message(WARNING "System not supported. Only Windows is supported for External_FTD2XX.cmake.")
  message(WARNING "FTD2XX should not be needed on your system.")
  return()
endif()

include(imstkAddExternalProject)

# Set FTD2XX_SOURCE_DIR and FTD2XX_PREFIX
imstk_define_external_dirs( FTD2XX )

# Directories and filenames specific to the FTD2XX archive layout
set(_dll_dir "i386")
set(_dll_name "ftd2xx.dll")
set(_lib_dir "i386")
set(_lib_name "ftd2xx.lib")
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(_dll_dir "amd64")
  set(_dll_name "ftd2xx64.dll")
  set(_lib_dir "amd64")
  set(_lib_name "ftd2xx.lib")
endif()

# Directories and filenames specific to the FTD2XX install layout
set(FTD2XX_DLL_DIR "bin")
set(FTD2XX_DLL_NAME "${_dll_name}")
set(FTD2XX_LIB_DIR "lib")
set(FTD2XX_LIB_NAME "${_lib_name}")

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(FTD2XX_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

set(copy_ftd2xx_headers_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_SOURCE_DIR}/ftd2xx.h
  ${FTD2XX_INSTALL_DIR}/include/ftd2xx/ftd2xx.h
  )
set(copy_ftd2xx_lib_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_SOURCE_DIR}/${_lib_dir}/${_lib_name}
  ${FTD2XX_INSTALL_DIR}/${FTD2XX_LIB_DIR}/${_lib_name}
  )
set(copy_ftd2xx_dll_command
  ${CMAKE_COMMAND} -E copy
  ${FTD2XX_SOURCE_DIR}/${_dll_dir}/${_dll_name}
  ${FTD2XX_INSTALL_DIR}/${FTD2XX_DLL_DIR}/${_dll_name}
  )

set(FTD2XX_INSTALL_COMMAND
  INSTALL_COMMAND
    COMMAND ${copy_ftd2xx_headers_command}
    COMMAND ${copy_ftd2xx_lib_command}
    COMMAND ${copy_ftd2xx_dll_command}
  )

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  # HS 2021-aug-08 Some dependencies expect the dll name to
  # be ftd2xx.dll rather than ftd2xx64.dll we're copying here
  # to the renamed version as the whole chain is built via 64 bit builds
  set(copy_ftd2xx_dll_command_rename
    ${CMAKE_COMMAND} -E copy
    ${FTD2XX_SOURCE_DIR}/${_lib_dir}/ftd2xx64.dll
    ${FTD2XX_INSTALL_DIR}/bin/ftd2xx.dll
    )

  list(APPEND FTD2XX_INSTALL_COMMAND
    COMMAND ${copy_ftd2xx_dll_command_rename}
    )
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
imstk_add_external_project( FTD2XX
  URL http://www.ftdichip.com/Drivers/CDM/CDM%20v2.12.18%20WHQL%20Certified.zip
  URL_MD5 e1a194765d7bcc58968d523484b74140
  DOWNLOAD_DIR ${FTD2XX_PREFIX}
  SOURCE_DIR ${FTD2XX_SOURCE_DIR}
  UPDATE_COMMAND ${SKIP_STEP_COMMAND}
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  ${FTD2XX_INSTALL_COMMAND}
  RELATIVE_INCLUDE_PATH "/"
  #VERBOSE
)
