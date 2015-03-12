find_path(SFML_INCLUDE_DIR
    SFML/Config.hpp
    )

find_library(SFML_audio_LIBRARY
  NAMES
    sfml-audio
    sfml-audio-d
    sfml-audio-s
    sfml-audio-s-d)
find_library(SFML_graphics_LIBRARY
  NAMES
    sfml-graphics
    sfml-graphics-d
    sfml-graphics-s
    sfml-graphics-s-d)
find_library(SFML_network_LIBRARY
  NAMES
    sfml-network
    sfml-network-d
    sfml-network-s
    sfml-network-s-d)
find_library(SFML_system_LIBRARY
  NAMES
    sfml-system
    sfml-system-d
    sfml-system-s
    sfml-system-s-d)
find_library(SFML_window_LIBRARY
  NAMES
    sfml-window
    sfml-window-d
    sfml-window-s
    sfml-window-s-d)

set(SFML_INCLUDE_DIRS "${SFML_INCLUDE_DIR}")
set(SFML_LIBRARIES "${SFML_audio_LIBRARY};${SFML_graphics_LIBRARY};${SFML_network_LIBRARY};${SFML_system_LIBRARY};${SFML_window_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SFML
  REQUIRED_VARS
    SFML_INCLUDE_DIR
    SFML_audio_LIBRARY
    SFML_graphics_LIBRARY
    SFML_network_LIBRARY
    SFML_system_LIBRARY
    SFML_window_LIBRARY)

mark_as_advanced(
    SFML_INCLUDE_DIR
    SFML_audio_LIBRARY
    SFML_graphics_LIBRARY
    SFML_network_LIBRARY
    SFML_system_LIBRARY
    SFML_window_LIBRARY)

if(SFML_FOUND)
  if(NOT TARGET sfml::audio)
    add_library(sfml::audio INTERFACE IMPORTED)
    set_target_properties(sfml::audio PROPERTIES
      INTERFACE_LINK_LIBRARIES "${SFML_audio_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}")
  endif()
  if(NOT TARGET sfml::graphics)
    add_library(sfml::graphics INTERFACE IMPORTED)
    set_target_properties(sfml::graphics PROPERTIES
      INTERFACE_LINK_LIBRARIES "${SFML_graphics_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}")
  endif()
  if(NOT TARGET sfml::network)
    add_library(sfml::network INTERFACE IMPORTED)
    set_target_properties(sfml::network PROPERTIES
      INTERFACE_LINK_LIBRARIES "${SFML_network_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}")
  endif()
  if(NOT TARGET sfml::system)
    add_library(sfml::system INTERFACE IMPORTED)
    set_target_properties(sfml::system PROPERTIES
      INTERFACE_LINK_LIBRARIES "${SFML_system_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}")
  endif()
  if(NOT TARGET sfml::window)
    add_library(sfml::window INTERFACE IMPORTED)
    set_target_properties(sfml::window PROPERTIES
      INTERFACE_LINK_LIBRARIES "${SFML_window_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}")
  endif()
endif()
