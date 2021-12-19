include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for VRPN
#-----------------------------------------------------------------------------

imstk_find_header(VRPN vrpn_Configure.h vrpn)
imstk_find_libary(VRPN vrpn)#Use same library for debug
imstk_find_libary(VRPN quat)
imstk_find_package(VRPN VRPN::VRPN)

#message(STATUS "VRPN include : ${VRPN_INCLUDE_DIRS}")
#message(STATUS "VRPN libraries : ${VRPN_LIBRARIES}")
