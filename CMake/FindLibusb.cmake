include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for Libusb
#-----------------------------------------------------------------------------

imstk_find_header(Libusb libusb-1.0/libusb.h)
imstk_find_libary(Libusb libusb-1.0 " ")#Use same library for debug
imstk_find_package(Libusb)

#message(STATUS "Libusb include : ${LIBUSB_INCLUDE_DIRS}")
#message(STATUS "Libusb libraries : ${LIBUSB_LIBRARIES}")
