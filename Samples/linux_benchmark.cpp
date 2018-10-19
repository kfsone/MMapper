// Author: Oliver "kfs1" Smith <oliver@kfs.org>
//
// This is a linux-only demonstration/test of mmap vs read.
// It takes two arguments:
//  mmaptest {read | mmap} <filename>
//
// It will then open the file and create a "checksum" of all the
// bytes in the file using either the normal read() method (with
// a small, 256 byte buffer) or using the mmap() alternative.
//
// Recommend you do something like time mmaptest read file ; time mmaptest mmapfile
// But give it a BIG file.
//
// Compile with GCC 4.6 or higher and
//  g++ -Wall -march=native -std=gnu++0x -O3 -flto -fwhole-program -o mmaptest mmaptest.cpp

#include <sys/mman.h>	// For Debian based Linux. Others just want <mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <wchar.h>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <inttypes.h>

// Takes a buffer and does some magic with so many bytes of it.
static uint64_t
_calculateChecksum(uint64_t checksum, const char* buffer, const size_t& size)
{
	// Calculate the bytes that express whole uint64s.
	const size_t wholeUint64_bytes = (size & ~(sizeof(uint64_t) - 1)) ;

	size_t i ;
	for ( i = 0 ; i < wholeUint64_bytes ; i += sizeof(uint64_t) )
	{
		checksum = ((checksum << 5) ^ *(unsigned long long*)(buffer + i)) ;
	}
	// Process the rest one byte at a time.
	for ( ; i < size ; ++i )
	{
		checksum = ((checksum << 5) ^ buffer[i]) ;
	}

	return checksum ;
}

int
main(int argc, const char* const argv[])
{
	if ( argc != 3 )
	{
		fprintf(stderr, "Usage: %s {read | mmap} <filename>\n", argv[0]) ;
		exit(1) ;
	}

	bool useMmap ;
	if ( strcmp(argv[1], "read") == 0 )
		useMmap = false ;
	else if ( strcmp(argv[1], "mmap") == 0 )
		useMmap = true ;
	else
	{
		fprintf(stderr, "Unknown mode: %s, expected 'read' or 'mmap'\n", argv[1]) ;
		exit(1) ;
	}

	// Open the file for level 2 io as you would normally.
	const char* const filename = argv[2] ;
	const int fd = open(filename, O_RDONLY) ;
	if ( fd < 0 )
	{
		fprintf(stderr, "Unable to open %s\n", filename) ;
		exit(1) ;
	}

	// Use fstat to quickly obtain the size of the open file handle.
	struct stat stats ;
	const int sr = fstat(fd, &stats) ;
	if ( sr < 0 )
	{
		fprintf(stderr, "Unable to stat %s: %s\n", filename, strerror(errno)) ;
		exit(1) ;
	}

	// Empty file? What are you trying to do to me?
	const size_t size = stats.st_size ;
	if ( size == 0 )
	{
		fprintf(stderr, "File %s is empty/has zero size.\n", filename) ;
		exit(1) ;
	}

	uint64_t checksum = 0ULL ;
	if ( useMmap )
	{
		////////// MMAP Code //////////

		// Ask the OS to provide an in-memory view of the data; which is
		// basically saying "load this file into buffers like you would,
		// but then give us direct access to the buffer memory".
		// Posix version.

		static const int mmFlags = MAP_FILE | MAP_PRIVATE ;
		const void* const ptr = mmap(NULL, size + 1, PROT_READ, mmFlags, fd, 0) ;
		if ( ptr == MAP_FAILED )
		{
			fprintf(stderr, "mmap() call failed with %s\n", strerror(errno)) ;
			exit(2) ;
		}

		const char* const buffer = static_cast<const char*>(ptr) ;

		// Process first part upto 8 bytes at a time.
		checksum = _calculateChecksum(checksum, buffer, size) ;
	}
	else
	{
		////////// READ Code //////////
		// The right buffer size can make a huge difference to
		// the performance of read() depending on particular
		// usage and deployment. But as a general rule of thumb,
		// you would want to work with multiples of the system's
		// page size.
		//
		// However, in this case I'm deliberately using a low
		// value based on typical values I've seen in many pieces
		// of code.
		static const size_t BufferSize = 256 ;
		char buffer[BufferSize] ;
		for ( ; ; )
		{
			const ssize_t bytesRead = read(fd, buffer, sizeof(buffer)) ;
			if ( bytesRead <= 0 )
				break ;

			checksum = _calculateChecksum(checksum, buffer, bytesRead) ;
		}
	}
	close(fd) ;

	printf("checksum of %s is %llu\n", filename, checksum) ;
}

