include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for glm
#-----------------------------------------------------------------------------

imstk_find_header(glm glm/glm.hpp)
imstk_find_header_package(glm)

#message(STATUS "glm include : ${GLM_INCLUDE_DIRS}")
