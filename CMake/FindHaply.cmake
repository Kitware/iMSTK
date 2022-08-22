include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for Assimp
#-----------------------------------------------------------------------------

imstk_find_header(Haply Inverse3.h Haply)
#set(Haply_INCLUDE_DIRS ${Haply_INCLUDE_DIRS}/Haply)
imstk_find_libary(Haply Haply.HardwareAPI)
imstk_find_package(Haply Haply::Haply)

# message(STATUS "Haply include : ${Haply_INCLUDE_DIRS}")
# message(STATUS "Haply libraries : ${Haply_LIBRARIES}")
