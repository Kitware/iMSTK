if(NOT WIN32)
  set(USE_SYSTEM_Libusb TRUE)  # Variable name expected by imstkAddExternalProject.cmake
endif()

include(imstkAddExternalProject)

# Set Libusb_SOURCE_DIR and Libusb_PREFIX
imstk_define_external_dirs( Libusb )

# Directories and filenames specific to the Libusb archive layout
set(_dll_dir "MS32/dll")
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(_dll_dir "MS64/dll")
endif()
set(_dll_name "libusb-1.0.dll")
set(_lib_dir "${_dll_dir}")
set(_lib_name "libusb-1.0.lib")

# Directories and filenames specific to the Libusb install layout
set(Libusb_INC_DIR "include/libusb-1.0")
set(Libusb_DLL_DIR "bin")
set(Libusb_DLL_NAME "${_dll_name}")
set(Libusb_LIB_DIR "lib")
set(Libusb_LIB_NAME "${_lib_name}")

#-----------------------------------------------------------------------------
# Set install commands
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "iMSTK")
  set(Libusb_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

set(copy_libusb_headers_command
  ${CMAKE_COMMAND} -E copy_directory
  ${Libusb_SOURCE_DIR}/include
  ${Libusb_INSTALL_DIR}/include
  )
set(copy_libusb_lib_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_SOURCE_DIR}/${_lib_dir}/${_lib_name}
  ${Libusb_INSTALL_DIR}/${Libusb_LIB_DIR}/${_lib_name}
  )
set(copy_libusb_dll_command
  ${CMAKE_COMMAND} -E copy
  ${Libusb_SOURCE_DIR}/${_dll_dir}/${_dll_name}
  ${Libusb_INSTALL_DIR}/${Libusb_DLL_DIR}/${_dll_name}
  )

set(Libusb_INSTALL_COMMAND
  INSTALL_COMMAND
    COMMAND ${copy_libusb_headers_command}
    COMMAND ${copy_libusb_lib_command}
    COMMAND ${copy_libusb_dll_command}
  )

if(USE_SYSTEM_Libusb)
  unset(Libusb_INCLUDE_DIR CACHE)
  unset(Libusb_LIBRARY_libusb-1.0-RELEASE CACHE)
  unset(Libusb_LIBRARY_libusb-1.0-DEBUG CACHE)

  find_package(Libusb REQUIRED)
endif()

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
  ${Libusb_INSTALL_COMMAND}
  RELATIVE_INCLUDE_PATH "${Libusb_INC_DIR}"
  #VERBOSE
)
