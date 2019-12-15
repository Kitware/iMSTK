include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for tbb
#-----------------------------------------------------------------------------

imstk_find_header(tbb tbb/tbb.h)
imstk_find_libary(tbb tbb)
imstk_find_package(tbb)

#message(STATUS "tbb include : ${TBB_INCLUDE_DIRS}")
#message(STATUS "tbb libraries : ${TBB_LIBRARIES}")