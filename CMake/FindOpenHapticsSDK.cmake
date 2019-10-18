include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for OpenHaptics SDK
#-----------------------------------------------------------------------------

imstk_find_header(OpenHapticsSDK HD/hd.h OpenHaptics)
imstk_find_libary(OpenHapticsSDK hd " ")#Use same library for debug
imstk_find_libary(OpenHapticsSDK hl " ")#Use same library for debug
imstk_find_libary(OpenHapticsSDK hdu " ")#Use same library for debug
imstk_find_libary(OpenHapticsSDK hlu " ")#Use same library for debug

imstk_find_package(OpenHapticsSDK)

message(STATUS "OpenHapticsSDK include : ${OPENHAPTICSSDK_INCLUDE_DIRS}")
message(STATUS "OpenHapticsSDK libraries : ${OPENHAPTICSSDK_LIBRARIES}")
