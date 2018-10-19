# KFS::MMapper

KFS::MMappedFile and KFS::FileHandle
Written by Oliver 'kfsone' Smith <oliver@kfs.org>, 2018 (Refactor)
Written by Oliver 'kfsone' Smith <oliver@kfs.org>, 2012 (Original)

# Description:

A multi-platform API wrapper for POSIX mmap/Win32 file mapping.

This is a refactor of a project I wrote many years ago; I got rid of the
text-file specific functions, dropped a lot of the member values that I
didn't really need (don't need to keep file handles open on either platform,
the mapping acts as its own retainer).

It also provides, incidentally, a simple cross-platform RAII wrapper for
file descriptors: KFS::FileHandle.


# Usage:

```
// Implementation of 'cat'.
#include "mmapedfile.h"

int main()
{
	KFS::MMappedFile mf { "/etc/motd" };
	if (!mf.isMapped())
		std::cerr << "ERROR: Failed to open /etc/motd\n";
	else
		std::cout << static_cast<const char*>(mf.begin());
}
```

# Samples:

Two samples are provided. Building them can be disabled by changing
the CMake variable `MMAPPER_BUILD_SAMPLES`, which is on by default.

## mmap_search:

Implements a simple "search" command that takes a needle to search
for and a list of files to search for it.

> mmap_search exchange *.cpp

This is implemented essentially as

```
	MMapedFile mf(filename);
	if (strstr(mf.begin(), needle) != 0)
		printf("Found in %s\n", filename);
```

## compare_read_mmap:

Takes a 'mode' and 'filename' parameter:

> compare_read_mmap read somebigfile.dat
> compare_read_mmap mmap somebigfile.dat

Both the `read(2)` and `MMapedFile` implementations are provided
for comparison. It could in theory be used to benchmark, but the
`read` code is deliberately hamstrung with a small buffer size
of 256 bytes. If you want to actually benchmark, change this
to 4096 bytes.

