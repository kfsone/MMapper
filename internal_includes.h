#pragma once

// Common includes used by the mmapper cpp files, not required for a 3rd party.

#if MMAPPER_API == MMAPPER_POSIX

# include <sys/mman.h>	// Some systems use mman.h instead.
# include <sys/types.h>
# include <sys/stat.h>
# include <errno.h>
# include <fcntl.h>
# include <unistd.h>

#elif MMAPPER_API == MMAPPER_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
# include <windows.h>

#else

#pragma error("Unrecognized platform.")

#endif


