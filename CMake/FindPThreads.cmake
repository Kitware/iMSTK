if(NOT WIN32)
  find_path(PTHREAD_INCLUDE_DIR
      pthread.h
      )

  find_library(PTHREAD_LIBRARY
    NAMES
      pthread
      libpthread
      )

  set(PTHREAD_INCLUDE_DIRS "${PTHREAD_INCLUDE_DIR}")
  set(PTHREAD_LIBRARIES "${PTHREAD_LIBRARY}")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PTHREAD
    REQUIRED_VARS
      PTHREAD_INCLUDE_DIR
      PTHREAD_LIBRARY)

  mark_as_advanced(
    PTHREAD_INCLUDE_DIR
    PTHREAD_LIBRARY)

  if(PTHREAD_FOUND AND NOT TARGET Threads::Threads)
    add_library(Threads::Threads INTERFACE IMPORTED)
    set_target_properties(Threads::Threads PROPERTIES
      INTERFACE_LINK_LIBRARIES "${PTHREAD_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${PTHREAD_INCLUDE_DIRS}")
  endif()
else()
  set(required)
  if(PThreads_FIND_REQUIRED)
    set(required REQUIRED)
  endif()
  find_package(Threads ${required})
endif()
