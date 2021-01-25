# - Find Boost
# 
# Copyright (c) 2016 Thiago Barroso Perrotta
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# This module finds if Boost is installed and determines where the
# executables are. It sets the following variables:
#
#  BOOST_FOUND : boolean            - system has Boost
#  BOOST_LIBRARIES : list(filepath) - the libraries needed to use Boost
#  BOOST_INCLUDE_DIRS : list(path)  - the Boost include directories
#
# If Boost is not found, this module downloads it according to the
# following variables:
#
#  BOOST_ROOT_DIR : path                - the Path where Boost will be installed on
#  BOOST_REQUESTED_VERSION : string     - the Boost version to be downloaded
#
# You can also specify its components:
#
#  find_package(Boost COMPONENTS program_options system)
#
# which are stored in Boost_FIND_COMPONENTS : list(string)
#
# You can also specify its behavior:
#
#  BOOST_USE_STATIC_LIBS : boolean (default: OFF)

if(NOT Boost_FIND_COMPONENTS)
    message(FATAL_ERROR "No COMPONENTS specified for Boost")
endif()

set(BOOST_USE_STATIC_LIBS false)

# Set the library prefix and library suffix properly.
if(BOOST_USE_STATIC_LIBS)
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_STATIC_LIBRARY_PREFIX})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
    set(LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

# Create a list(string) for the build command (e.g. --with-program_options;--with-system)
# and assigns it to BOOST_COMPONENTS_FOR_BUILD
foreach(component ${Boost_FIND_COMPONENTS})
    list(APPEND BOOST_COMPONENTS_FOR_BUILD --with-${component})
endforeach()

# Create a string for the first component (e.g. boost_program_options)
# and assigns it to Boost_FIND_COMPONENTS
list(GET Boost_FIND_COMPONENTS 0 BOOST_FIRST_COMPONENT)
set(BOOST_FIRST_COMPONENT "boost_${BOOST_FIRST_COMPONENT}")

include(FindPackageHandleStandardArgs)

macro(DO_FIND_BOOST_SYSTEM)
    find_path(BOOST_INCLUDE_DIR boost/config.hpp
        PATHS /usr/local/include /usr/include
        )
    find_library(BOOST_LIBRARY
        NAMES ${BOOST_FIRST_COMPONENT}
        PATHS /usr/local/lib /usr/lib
        )
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG
        BOOST_INCLUDE_DIR BOOST_LIBRARY
        )
    set(BOOST_LIBRARIES ${BOOST_LIBRARY})
    set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
    mark_as_advanced(BOOST_LIBRARIES BOOST_INCLUDE_DIRS)
endmacro()

macro(DO_FIND_BOOST_ROOT)
    if(NOT BOOST_ROOT_DIR)
        message(STATUS "BOOST_ROOT_DIR is not defined, using binary directory.")
        set(BOOST_ROOT_DIR ${CURRENT_CMAKE_BINARY_DIR} CACHE PATH "")
    endif()

    find_path(BOOST_INCLUDE_DIR boost/config.hpp ${BOOST_ROOT_DIR}/include)
    find_library(BOOST_LIBRARY ${BOOST_FIRST_COMPONENT} HINTS ${BOOST_ROOT_DIR}/lib)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG
        BOOST_INCLUDE_DIR BOOST_LIBRARY
        )
    set(BOOST_LIBRARIES ${BOOST_LIBRARY})
    set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
    mark_as_advanced(BOOST_LIBRARIES BOOST_INCLUDE_DIRS)
endmacro()

macro(DO_FIND_BOOST_DOWNLOAD)
    if(NOT BOOST_REQUESTED_VERSION)
        message(FATAL_ERROR "BOOST_REQUESTED_VERSION is not defined.")
    endif()

    string(REPLACE "." "_" BOOST_REQUESTED_VERSION_UNDERSCORE ${BOOST_REQUESTED_VERSION})

    set(BOOST_MAYBE_STATIC)
    if(BOOST_USE_STATIC_LIBS)
        set(BOOST_MAYBE_STATIC "link=static")
    endif()

    include(ExternalProject)
    ExternalProject_Add(
        Boost
        #URL https://downloads.sourceforge.net/project/boost/boost/${BOOST_REQUESTED_VERSION}/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.zip
        URL https://dl.bintray.com/boostorg/release/${BOOST_REQUESTED_VERSION}/source/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.zip
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_ROOT_DIR}
        BUILD_COMMAND ./b2 ${BOOST_MAYBE_STATIC} --prefix=${BOOST_ROOT_DIR} ${BOOST_COMPONENTS_FOR_BUILD} install
        BUILD_IN_SOURCE true
        INSTALL_COMMAND ""
        INSTALL_DIR ${BOOST_ROOT_DIR}
        )

    ExternalProject_Get_Property(Boost install_dir)
    set(BOOST_INCLUDE_DIRS ${install_dir}/include)

    macro(libraries_to_fullpath varname)
        set(${varname})
        foreach(component ${Boost_FIND_COMPONENTS})
            list(APPEND ${varname} ${BOOST_ROOT_DIR}/lib/${LIBRARY_PREFIX}boost_${component}${LIBRARY_SUFFIX})
        endforeach()
    endmacro()
    libraries_to_fullpath(BOOST_LIBRARIES)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG
        BOOST_INCLUDE_DIRS BOOST_LIBRARIES
        )
    mark_as_advanced(BOOST_LIBRARIES BOOST_INCLUDE_DIRS)
endmacro()

if(NOT BOOST_FOUND)
    DO_FIND_BOOST_ROOT()
endif()

if(NOT BOOST_FOUND)
    DO_FIND_BOOST_SYSTEM()
endif()

if(NOT BOOST_FOUND)
    DO_FIND_BOOST_DOWNLOAD()
endif()

