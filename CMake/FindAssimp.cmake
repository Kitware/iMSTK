include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for Assimp
#-----------------------------------------------------------------------------

imstk_find_header(Assimp assimp/Importer.hpp)
imstk_find_libary(Assimp assimp)
imstk_find_package(Assimp)

#message(STATUS "Assimp include : ${ASSIMP_INCLUDE_DIRS}")
#message(STATUS "Assimp libraries : ${ASSIMP_LIBRARIES}")

