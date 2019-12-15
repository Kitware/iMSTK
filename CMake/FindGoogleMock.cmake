include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for GoogleMock
#-----------------------------------------------------------------------------

imstk_find_header(GoogleMock gmock/gmock.h)
imstk_find_libary(GoogleMock gmock)
imstk_find_libary(GoogleMock gmock_main)
imstk_find_package(GoogleMock)

#message(STATUS "GoogleMock include : ${GOOGLEMOCK_INCLUDE_DIRS}")
#message(STATUS "GoogleMock libraries : ${GOOGLEMOCK_LIBRARIES}")
