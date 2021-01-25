#[=======================================================================[.rst:
Findlibusb
-------

Finds the libusb library. Specifically finds libusb-1.0

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libusb::libusb``
  The libusb library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``libusb_FOUND``
  True if the system has the Foo library.
``libusb_INCLUDE_DIRS``
  Include directories needed to use Foo.
``libusb_LIBRARIES``
  Libraries needed to link to Foo.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``libusb_INCLUDE_DIR``
  The directory containing ``foo.h``.
``libusb_LIBRARY``
  The path to the Foo library.

#]=======================================================================]



# - Try to find libusb-1.0
# Once done this will define
#
#  libusb_FOUND - system has libusb
#  libusb_INCLUDE_DIRS - the libusb include directory
#  libusb_LIBRARIES - Link these to use libusb
#  libusb_DEFINITIONS - Compiler switches required for using libusb
#
#  Adapted from cmake-modules Google Code project
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  (Changes for libusb) Copyright (c) 2008 Kyle Machulis <kyle@nonpolynomial.com>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
#
# CMake-Modules Project New BSD License
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the CMake-Modules Project nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

file(GLOB_RECURSE extern_file ${PROJECT_SOURCE_DIR}/third_party/*libusb.h)
if (extern_file)
  # strip file name
  get_filename_component(extern_lib_tmp_path ${extern_file} PATH)
  # strip 'libusb-1.0' path component
  get_filename_component(extern_lib_include_path ${extern_lib_tmp_path} PATH)
  # strip down to main lib dir
  get_filename_component(extern_lib_path ${extern_lib_include_path} PATH)
  MESSAGE(STATUS "Found libusb in 'extern' subfolder: ${extern_lib_path}")
endif (extern_file)

if (libusb_LIBRARIES AND libusb_INCLUDE_DIRS)
  # in cache already
  set(libusb_FOUND TRUE)
  if (libusb_FOUND AND NOT TARGET libusb::libusb)
    add_library(libusb::libusb UNKNOWN IMPORTED)
    set_target_properties(libusb::libusb PROPERTIES
      IMPORTED_LOCATION "${libusb_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${libusb_INCLUDE_DIR}"
      )
  endif ()
else (libusb_LIBRARIES AND libusb_INCLUDE_DIRS)
  find_path(libusb_INCLUDE_DIR
    NAMES
    libusb.h
    PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ${extern_lib_include_path}
    PATH_SUFFIXES
    libusb-1.0
    )

  # determine if we run a 64bit compiler or not
  set(bitness 32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(bitness 64)
  endif ()

  find_library(libusb_LIBRARY
    NAMES
    usb-1.0 libusb-1.0
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ${extern_lib_path}/MS${bitness}/dll
    )

  # set path to DLL for later installation
  IF (WIN32 AND libusb_LIBRARY)
    # strip file name
    get_filename_component(libusb_lib_path ${libusb_LIBRARY} PATH)
    if (EXISTS ${libusb_lib_path}/libusb-1.0.dll)
      SET(libusb_DLL ${libusb_lib_path}/libusb-1.0.dll)
    else ()
      #strip last directory level
      get_filename_component(libusb_lib_path ${libusb_lib_path} PATH)
      if (EXISTS ${libusb_lib_path}/dll/libusb-1.0.dll)
        SET(libusb_DLL ${libusb_lib_path}/dll/libusb-1.0.dll)
      endif (EXISTS ${libusb_lib_path}/dll/libusb-1.0.dll)
    endif (EXISTS ${libusb_lib_path}/libusb-1.0.dll)
  endif (WIN32 AND libusb_LIBRARY)


  set(libusb_INCLUDE_DIRS
    ${libusb_INCLUDE_DIR}
    )
  set(libusb_LIBRARIES
    ${libusb_LIBRARY}
    )

  if (libusb_INCLUDE_DIRS AND libusb_LIBRARIES)
    set(libusb_FOUND TRUE)
  endif (libusb_INCLUDE_DIRS AND libusb_LIBRARIES)

  if (libusb_FOUND AND NOT TARGET libusb::libusb)
    add_library(libusb::libusb UNKNOWN IMPORTED)
    set_target_properties(libusb::libusb PROPERTIES
      IMPORTED_LOCATION "${libusb_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${libusb_INCLUDE_DIR}"
      )
  endif ()

  if (libusb_FOUND)
    if (NOT libusb_FIND_QUIETLY)
      message(STATUS "Found libusb-1.0:")
      message(STATUS " - Includes: ${libusb_INCLUDE_DIRS}")
      message(STATUS " - Libraries: ${libusb_LIBRARIES}")
    endif (NOT libusb_FIND_QUIETLY)
  else (libusb_FOUND)
    if (libusb_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libusb")
    endif (libusb_FIND_REQUIRED)
  endif (libusb_FOUND)

  # show the libusb_INCLUDE_DIRS and libusb_LIBRARIES variables only in the advanced view
  mark_as_advanced(libusb_INCLUDE_DIRS libusb_LIBRARIES)

endif (libusb_LIBRARIES AND libusb_INCLUDE_DIRS)