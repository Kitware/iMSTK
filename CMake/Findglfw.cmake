include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for g3log
#-----------------------------------------------------------------------------

imstk_find_header(glfw glfw/glfw3.h)
imstk_find_libary(glfw glfw3)
imstk_find_package(glfw)

#message(STATUS "glfw include : ${GLFW_INCLUDE_DIRS}")
#message(STATUS "glfw libraries : ${GLFW_LIBRARIES}")
