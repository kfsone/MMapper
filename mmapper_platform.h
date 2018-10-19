#pragma once

// mmapper_platform.h -- Platform/build configuration settings.
// Author: Oliver "kfsone" Smith 2012, 2018 <oliver@kfs.org>
// Redistribution and re-use fully permitted contingent on inclusion of these 3 lines in copied- or derived- works.

// Undefine this if you want exceptions in the ctor and map file.
#define MMAPPER_NO_THROW
#ifdef MMAPPER_NO_THROW
# define MMAPPER_MAYBE_NOEXCEPT noexcept
#else
# define MMAPPER_MAYBE_NOEXCEPT
#endif


// Platform includes
#if (defined(WIN32) || defined(_MSC_VER)) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <unistd.h>
#endif


// Common includes.
#include <cwchar>
#include <string>


// Which API we use is largely based on the platform we're building under.
// I'd put these in the namespace, but they're #defines.
#define MMAPPER_WIN32 0
#define MMAPPER_POSIX 1


namespace KFS
{

#if (defined(WIN32) || defined(_MSC_VER)) && !defined(__CYGWIN__)

// Windows

	#define MMAPPER_API MMAPPER_WIN32

	using file_handle_t = HANDLE;

	// Visual studio has a property to tell us if we should be using unicode.
	#if defined(UNICODE)
		#define MMAPPER_USE_UNICODE
	#endif

	#define	MMAPPER_PATH_SEP   '\\'

#elif defined(__GNUC__) || defined(__clang__) || (defined(__APPLE__) && defined(__MACH__))

	// Mac, Linux, POSIX...

	#define MMAPPER_API MMAPPER_POSIX

	using file_handle_t = int;
	#define INVALID_HANDLE_VALUE	(-1)

	#undef MMAPPER_USE_UNICODE

	#define MMAPPER_PATH_SEP	'/'

#endif


// Find the appropriate character type.
#if defined(MMAPPER_USE_UNICODE)
	using filename_char_t = wchar_t;	///TODO: What about utf-8 in 17?
#else
	using filename_char_t = char;
#endif
	constexpr filename_char_t c_PathSeparator{ MMAPPER_PATH_SEP };

#if MMAPPER_API != MMAPPER_WIN32
	// At time of writing, Linux/BSD kernels don't provide widecharacter open()s.
	static_assert(sizeof(filename_char_t) == 1, "wide-character filenames not supported on this platform");
#endif

	// Creates a string type of the appropriate base character.
	using filename_str_t = std::basic_string<filename_char_t>;

}
