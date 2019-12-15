include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for GoogleTest
#-----------------------------------------------------------------------------

imstk_find_header(GoogleTest gtest/gtest.h)
imstk_find_libary(GoogleTest gtest)
imstk_find_libary(GoogleTest gtest_main)
imstk_find_package(GoogleTest)

#message(STATUS "GoogleTest include : ${GOOGLETEST_INCLUDE_DIRS}")
#message(STATUS "GoogleTest libraries : ${GOOGLETEST_LIBRARIES}")

