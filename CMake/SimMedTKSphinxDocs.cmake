function(simmedtk_add_doc sphinxTargetName)
  set(options)
  set(oneValueArgs DESTINATION SOURCE_DIR BUILD_DIR)
  set(multiValueArgs DEPENDS FIGURES)
  cmake_parse_arguments(sphinx "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  if (NOT sphinx_SOURCE_DIR)
    set(sphinx_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}") # Reasonable default
  endif()
  if (NOT sphinx_BUILD_DIR)
    set(sphinx_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}") # Reasonable default
  endif()
  # Generate HTML version of docs
  set(sphinx_HTML_TOP "${CMAKE_CURRENT_BINARY_DIR}/${sphinx_BUILD_DIR}/html/index.html")
  add_custom_command(
    OUTPUT "${sphinx_HTML_TOP}"
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/conf.py
      ${sphinx_DEPENDS}
      ${figureList}
      COMMAND ${SPHINX_EXECUTABLE}
    ARGS
      -b html
      "${sphinx_SOURCE_DIR}"
      "${sphinx_BUILD_DIR}/html"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating HTML for ${sphinxTargetName}"
  )
  add_custom_target(doc-${sphinxTargetName} DEPENDS "${sphinx_HTML_TOP}")
  if (sphinx_DESTINATION)
    install(
      DIRECTORY "${sphinx_BUILD_DIR}/html/"
      DESTINATION "${sphinx_DESTINATION}"
      COMPONENT Development)
    install(
      FILES ${figureList}
      DESTINATION "${sphinx_DESTINATION}/figures"
      COMPONENT Development)
  endif()
endfunction()
