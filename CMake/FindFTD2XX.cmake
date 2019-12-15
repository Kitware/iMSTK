include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for FTD2XX
#-----------------------------------------------------------------------------

imstk_find_header(FTD2XX ftd2xx.h ftd2xx)
imstk_find_libary(FTD2XX ftd2xx " ")#Use same library for debug)
imstk_find_package(FTD2XX)

#message(STATUS "FTD2XX include : ${FTD2XX_INCLUDE_DIRS}")
#message(STATUS "FTD2XX libraries : ${FTD2XX_LIBRARIES}")
