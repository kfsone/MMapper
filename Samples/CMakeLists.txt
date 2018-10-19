# Build the samples for MMapper
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

MESSAGE(STATUS "Samples")

INCLUDE_DIRECTORIES(${MMAPPER_INCLUDE_PATHS})

# Simple demonstration of how to use mmapper.
ADD_EXECUTABLE(
	mmap_search

	mmap_search.cpp
)
TARGET_LINK_LIBRARIES(mmap_search mmapper)

# A small tool that compares read/mmap by
# performing checksums on files.
ADD_EXECUTABLE(
	compare_read_mmap

	compare_read_mmap.cpp
)
SET_TARGET_PROPERTIES(
	compare_read_mmap

	PROPERTIES

	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)
TARGET_LINK_LIBRARIES(compare_read_mmap mmapper)
