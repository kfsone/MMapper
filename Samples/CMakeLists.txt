# Build the samples for MMapper
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT("MMapperSamples")

INCLUDE_DIRECTORIES(${MMAPPER_INCLUDE_PATHS})

# Simple demonstration of how to use mmapper.
ADD_EXECUTABLE(
	mmap_search

	mmap_search.cpp
)
TARGET_LINK_LIBRARIES(mmap_search mmapper)

# Linux et al get a benchmark example too.
IF (!WIN32)
	ADD_EXECUTABLE(
		linux_benchmark

		linux_benchmark.cpp
	)
	TARGET_LINK_LIBRARIES(live_benchmark mmapper)
ENDIF ()
