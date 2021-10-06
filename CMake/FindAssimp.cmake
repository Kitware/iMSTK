include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for Assimp
#-----------------------------------------------------------------------------

imstk_find_header(Assimp assimp/Importer.hpp)
imstk_find_libary(Assimp assimp)
imstk_find_package(Assimp Assimp::Assimp)

#message(STATUS "Assimp include : ${Assimp_INCLUDE_DIRS}")
#message(STATUS "Assimp libraries : ${Assimp_LIBRARIES}")

