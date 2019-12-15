include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for glm
#-----------------------------------------------------------------------------

imstk_find_header(PThreads pthread.h pthread)
imstk_find_libary(PThreads pthread)
imstk_find_package(PThreads Threads::Threads)

#message(STATUS "PThreads include : ${PTHREADS_INCLUDE_DIRS}")
#message(STATUS "PThreads libraries : ${PTHREADS_LIBRARIES}")
