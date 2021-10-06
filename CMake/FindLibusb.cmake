include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for Libusb
#-----------------------------------------------------------------------------

set(USE_SYSTEM)
if(NOT WIN32)
  set(USE_SYSTEM "USE_SYSTEM")
endif()

imstk_find_header(Libusb libusb-1.0/libusb.h ${USE_SYSTEM})
imstk_find_libary(Libusb libusb-1.0 " " ${USE_SYSTEM})#Use same library for debug
imstk_find_package(Libusb)

#message(STATUS "Libusb include : ${Libusb_INCLUDE_DIRS}")
#message(STATUS "Libusb libraries : ${Libusb_LIBRARIES}")
