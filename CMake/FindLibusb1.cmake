# - try to find libusb-1 library
#
# Cache Variables: (probably not for direct use in your scripts)
#  LIBUSB1_LIBRARY
#  LIBUSB1_INCLUDE_DIR
#
# Non-cache variables you should use in your CMakeLists.txt:
#  LIBUSB1_LIBRARIES
#  LIBUSB1_INCLUDE_DIRS
#  LIBUSB1_FOUND - if this is not true, do not attempt to use this library
#
# Requires these CMake modules:
#  ProgramFilesGlob
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


set(LIBUSB1_ROOT_DIR
	"${LIBUSB1_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for libusb-1")

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		if(MSVC)
			set(_lib_suffixes MS64/static)
		endif()
	elseif(COMPILER_IS_GNUCXX)
		set(_lib_suffixes MinGW64/static)
	else()
        if(MSVC)
            set(_lib_suffixes MS32/static)
        elseif(COMPILER_IS_GNUCXX)
            set(_lib_suffixes MinGW32/static)
        endif()
	endif()
else()
	set(_lib_suffixes)
	find_package(PkgConfig QUIET)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(PC_LIBUSB1 libusb-1.0)
	endif()
endif()

find_library(LIBUSB_LIBRARY
    NAMES
        usb-1.0
        libusb-1.0
    PATHS
        C:/LibUSB-Win32
        "${LIBUSB1_ROOT_DIR}"
    PATH_SUFFIXES
        "${_lib_suffixes}"
)

find_path(LIBUSB_INCLUDE_DIR
	NAMES
		libusb.h
	PATHS
        C:/LibUSB-Win32
		"${LIBUSB1_ROOT_DIR}"
	PATH_SUFFIXES
		include/libusb-1.0
		include
		libusb-1.0
	)

# In windows find_* cant find the paths when a digit is included in the name
set(LIBUSB1_LIBRARY ${LIBUSB_LIBRARY})
set(LIBUSB1_INCLUDE_DIR ${LIBUSB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libusb1
	REQUIRED_VARS
		LIBUSB1_LIBRARY
		LIBUSB1_INCLUDE_DIR
)

if(LIBUSB1_FOUND)
	set(LIBUSB1_LIBRARIES "${LIBUSB1_LIBRARY}")

	set(LIBUSB1_INCLUDE_DIRS "${LIBUSB1_INCLUDE_DIR}")

	mark_as_advanced(LIBUSB1_ROOT_DIR)
endif()

mark_as_advanced(LIBUSB1_INCLUDE_DIR LIBUSB1_LIBRARY)
