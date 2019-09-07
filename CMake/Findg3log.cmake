include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for g3log
#-----------------------------------------------------------------------------

imstk_find_header(g3log g3log/g3log.hpp)
imstk_find_libary(g3log g3logger)
imstk_find_package(g3log)

#message(STATUS "g3log include : ${G3LOG_INCLUDE_DIRS}")
#message(STATUS "g3log libraries : ${G3LOG_LIBRARIES}")

#set(g3log_PLATFORM_LINK_LIBRARIES)
#if(WIN32)
#  set(g3log_PLATFORM_LINK_LIBRARIES dbghelp)
#endif()
