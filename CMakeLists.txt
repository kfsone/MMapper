CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (MMapper)

# Requires C++14 or higher.
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(MMAPPER_BUILD_SAMPLES "Build MMapper sample programs" ON)

SET(MMAPPER_LIB_SRCS

	mmapper.cpp
		mmapper.h
		mmapper_platform.h
		internal_includes.h

	filehandle.cpp
		filehandle.h
		mmapper_platform.h
		internal_includes.h
)

SET(MMAPPER_INCLUDE_PATHS

	${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_LIBRARY(
	mmapper

	${MMAPPER_LIB_SRCS}
)

IF(MMAPPER_BUILD_SAMPLES)
	ADD_SUBDIRECTORY(Samples)
ENDIF()
