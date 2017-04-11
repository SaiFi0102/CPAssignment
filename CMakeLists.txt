CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

#Force x64 native toolchain for Visual Studio x64 build
IF("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 14 2015 Win64")
    SET(CMAKE_GENERATOR_TOOLSET "v140</PlatformToolset><PreferredToolArchitecture>x64</PreferredToolArchitecture><PlatformToolset>v140")
	MESSAGE("Setting PreferredToolArchitecture=x64")
ENDIF()

PROJECT(CPPr)

SET(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CPPr_SOURCE_DIR}/cmake
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

#Find Wt
FIND_PACKAGE(Wt REQUIRED)
INCLUDE_DIRECTORIES(${Wt_INCLUDE_DIR})

#Include Boost
INCLUDE(cmake/WtFindBoost-cmake.txt)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})
LINK_DIRECTORIES(${BOOST_LIB_DIRS})

#Suppressing some boost related warnings
IF(WIN32)
	ADD_DEFINITIONS(
	  -D_CRT_SECURE_NO_WARNINGS
	  -D_SCL_SECURE_NO_WARNINGS
	)
ENDIF(WIN32)

SUBDIRS(src)