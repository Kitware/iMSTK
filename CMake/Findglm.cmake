include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for glm
#-----------------------------------------------------------------------------

imstk_find_header(glm glm/glm.hpp)
imstk_find_libary(glm g3logger)
imstk_find_package(glm)

#message(STATUS "glm include : ${GLM_INCLUDE_DIRS}")
#message(STATUS "glm libraries : ${GLM_LIBRARIES}")
