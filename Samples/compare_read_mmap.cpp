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
// Recommend you do something like time mmaptest read file; time mmaptest mmapfile
// But give it a BIG file.

// Don't need Microsoft warnings about ISO names for this demonstration.
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>

#include <fcntl.h>

#include "mmapper.h"
#include "filehandle.h"


#if defined(WIN32) && defined(_MSC_VER)

	#include <io.h>

	#define open(...) _open(__VA_ARGS__)
	#define read(...) _read(__VA_ARGS__)
	#define fstat(...) _fstat64(__VA_ARGS__)
	#define close(...) _close(__VA_ARGS__)

	using stat_buf = struct __stat64;

#else

	#include <sys/stat.h>

	#define O_BINARY 0

	using stat_buf = struct stat;

#endif


#include "../3rdParty/xxhash.hpp"

	
	template<typename... Args>
void die(const char* reason, Args&&... args)
{
	std::cerr << "ERROR: " << reason;
	(std::cerr << ... << args);
	std::cerr << std::endl;
	exit(1);
}

//////////////////////////////////////////////////////////////////////////////
// Borrowed from ...


int main(int argc, const char* const argv[])
{
	if ( argc != 3 )
		die("Usage: ", argv[0], " {read | mmap} <filename>");

	const char* mode = argv[1];
	bool useMmap;
	if ( strcmp(mode, "read") == 0 )
		useMmap = false;
	else if ( strcmp(mode, "mmap") == 0 )
		useMmap = true;
	else
		die("Unknown mode: ", argv[1], ". Expecting 'read' or 'mmap'");

	// We calculate a checksum either way.
	const char* filename = argv[2];
	uint64_t checksum{ 0 };
	uint64_t size{0};

	xxh::hash_state_t<64> hash_stream;
	if (!useMmap)  // I put this there to show you what the normal pattern is first.
	{
		////////// READ Code //////////
		// The right buffer size can make a huge difference to the
		// performance of read() depending on particular usage and
		// deployment. But as a general rule of thumb, you would want
		// to work with multiples of the system's page size.
		//
		// However, in this case I'm deliberately using a low value
		// based on typical values I've seen in many pieces of code:
		// 	256 bytes
		// (which is usually 1/16th of the page size :(

		int fd = open(filename, O_RDONLY | O_BINARY);
		if (fd < 0)
			die("Could not open file", filename);

		// Look up the size of the file.
		stat_buf st_buf;
		if (fstat(fd, &st_buf) < 0)
			die("Unable to get file size");
		size = st_buf.st_size;
		if (size <= 0)
			die("File is 0 bytes long.");

		// Accumulate the checksum over each block of data we read.
		static const size_t BufferSize = 256;
		char buffer[BufferSize];
		for ( ; ; )
		{
			auto bytesRead = read(fd, buffer, sizeof(buffer));
			if ( bytesRead <= 0 )
				break;
			hash_stream.update(buffer, bytesRead);
		}

		close(fd);
	}
	else // (useMmap)
	{
		////////// MMAP Code //////////
		// As the OS reads from a file, it generally loads data in
		// pages rather than byte-by-byte, to reduce the amount of
		// disk operations it has to execute.
		//
		// However, when we call 'read' we pass it a pointer to
		// our own buffer, so it *has* to copy the data from it's
		// page-based buffer into our buffer.
		//
		// This is why people often read data in smaller chunks,
		// like the 256 example above, because they think it will
		// reduce the damage done to cache coherence.
		// 
		// Either way - copying all that data is bad. "MMap" (or
		// creating a file mapping on Windows) allows the OS to
		// give you access to the file data through virtual addressing.

		// Create the MMappedFile object by opening the file,
		// and using the native memory-mapping API to produce a
		// pointer to the disk data.
		KFS::MMappedFile mf(filename);

		// Make sure it worked.
		if (!mf.isMapped())
			die("Failed to map file");

		// That's it. We can pass the entire file to the function
		// and the OS will worry about paging/loading the file as
		// required.
		//
		// The OS may even be able to make memory-management
		// decisions for us based on our usage patterns.
		hash_stream.update(mf.begin(), mf.size());

		size = mf.size();
	}

	// Try both versions and compare the checksums and the timing.
	std::cout << filename << ":" << mode << ": size " << size << " bytes, checksum " << std::hex << hash_stream.digest() << "\n";
}

