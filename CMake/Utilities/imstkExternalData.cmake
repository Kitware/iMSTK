include(ExternalData)

if(NOT ExternalData_OBJECT_STORES)
  # Use ExternalData_OBJECT_STORES from environment as default.
  set(ExternalData_OBJECT_STORES_DEFAULT "")
  if(DEFINED "ENV{ExternalData_OBJECT_STORES}")
    file(TO_CMAKE_PATH "$ENV{ExternalData_OBJECT_STORES}" ExternalData_OBJECT_STORES_DEFAULT)
  endif()
endif()

set(ExternalData_OBJECT_STORES "${ExternalData_OBJECT_STORES_DEFAULT}" CACHE STRING
  "Semicolon-separated list of local directories holding data objects in the layout %(algo)/%(hash).")
mark_as_advanced(ExternalData_OBJECT_STORES)
if(NOT ExternalData_OBJECT_STORES)
  set(ExternalData_OBJECT_STORES "${CMAKE_BINARY_DIR}/ExternalData/Objects")
  file(MAKE_DIRECTORY "${ExternalData_OBJECT_STORES}")
endif()
list(APPEND ExternalData_OBJECT_STORES
  # Local data store populated by the iMSTK pre-commit hook
  "${CMAKE_SOURCE_DIR}/.ExternalData"
  )

set(ExternalData_URL_TEMPLATES "" CACHE STRING
  "Additional URL templates for the ExternalData CMake script to look for testing data. E.g.
file:///var/bigharddrive/%(algo)/%(hash)")
mark_as_advanced(ExternalData_URL_TEMPLATES)
list(APPEND ExternalData_URL_TEMPLATES
  # Data published on Girder
  "https://data.kitware.com:443/api/v1/file/hashsum/%(algo)/%(hash)/download")

# Tell ExternalData commands to transform raw files to content links.
# TODO: Condition this feature on presence of our pre-commit hook.
set(ExternalData_LINK_CONTENT SHA512)

# Emscripten currently has difficulty reading symlinks.
if(EMSCRIPTEN)
  set(ExternalData_NO_SYMLINKS 1)
endif()

# Match series of the form <base>.<ext>, <base>.<n>.<ext> such that <base> may
# end in a (test) number that is not part of any series numbering.
set(ExternalData_SERIES_PARSE "()(\\.[^./]*)$")
set(ExternalData_SERIES_MATCH "(\\.[0-9]+)?")

# Sometimes we want to download very large files.
set(ExternalData_TIMEOUT_INACTIVITY 60)
set(ExternalData_SOURCE_ROOT "${iMSTK_SOURCE_DIR}/Data")
set(ExternalData_BINARY_ROOT "${CMAKE_INSTALL_PREFIX}/data/")

# Define the path to the data root directory
add_definitions( -DiMSTK_DATA_ROOT=\"${ExternalData_BINARY_ROOT}\")

# Function to upload data from list of data file
function(imstk_add_data target)
  # Download data
  foreach(file IN LISTS ARGN)
    set(datalist "${datalist} DATA{${ExternalData_SOURCE_ROOT}/${file}}")
  endforeach()

  ExternalData_expand_arguments(${target}ExternalData
    COMMAND ${datalist}
  )

  # Add a build target to populate the real data.
  ExternalData_Add_Target(${target}ExternalData)
  
  #-----------------------------------------------------------------------------
  # Add the target to Examples/Data folder
  #-----------------------------------------------------------------------------
  SET_TARGET_PROPERTIES (${target}ExternalData PROPERTIES FOLDER Examples/Data)
endfunction()
