include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for LibNifalcon
#-----------------------------------------------------------------------------

imstk_find_header(LibNiFalcon falcon/core/FalconCore.h)
imstk_find_libary(LibNiFalcon nifalcon)
imstk_find_package(LibNiFalcon)

#message(STATUS "LibNiFalcon include : ${LibNiFalcon_INCLUDE_DIRS}")
#message(STATUS "LibNiFalcon libraries : ${LibNiFalcon_LIBRARIES}")

