include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for glm
#-----------------------------------------------------------------------------

imstk_find_header(gli gli/gli.hpp)
imstk_find_header_package(gli)

#message(STATUS "gli include : ${GLI_INCLUDE_DIRS}")