A multi-platform API wrapper for POSIX mmap/Win32 file mapping.

This is a refactor of a project I wrote many years ago; I got rid of the
text-file specific functions, dropped a lot of the member values that I
didn't really need (don't need to keep file handles open on either platform,
the mapping acts as its own retainer).

Usage:

	{
		KFS::MMappedFile mf { "/etc/motd" };
		std::cout << static_cast<const char*>(mf.begin());
	}

