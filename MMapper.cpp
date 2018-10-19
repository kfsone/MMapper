// MMapper -- Cross-platform (Win/Posix) mmap interface.
// Author: Oliver "kfsone" Smith 2012, 2018 <oliver@kfs.org>
// Redistribution and re-use fully permitted contingent on inclusion of these 3 lines in copied- or derived- works.

#include "mmapper_platform.h"

#include <stdexcept>
#include <cstring>

#include "mmapper.h"
#include "filehandle.h"
#include "internal_includes.h"


namespace KFS
{

	//////////////////////////////////////////////////////////////////////
	// Populate filename with the combination of file and directory name.

	static filename_str_t
	_populateFilename(filename_str_t lhs_, filename_str_t rhs_)
	{
		filename_str_t into{ lhs_ };
		if (!into.empty() && into.back() != c_PathSeparator)
			into += c_PathSeparator;
		into += rhs_;
		return into;
	}


	//////////////////////////////////////////////////////////////////////
	// Constructor (POSIX + Windows versions combined)
	// Note: POSIX doesn't support wchar_t filenames.

	MMappedFile::MMappedFile(filename_str_t filename_, filename_str_t dirname_) MMAPPER_MAYBE_NOEXCEPT
	{
		bool mapped = mapFile(filename_, dirname_);

		// If MMAPPER_NO_THROW is defined, don't throw.
	#ifndef MMAPPER_NO_THROW
		if (!mapped)
		{
	#if IO_MAP_TYPE == _IMT_WINDOWS
			throw std::runtime_error(strerror(GetLastError()));
	#else
			throw std::runtime_error(strerror(errno));
	#endif
		}
	#endif
	}


	//////////////////////////////////////////////////////////////////////
	// Destructor.

	MMappedFile::~MMappedFile() noexcept
	{
		unmapFile();
	}


	//////////////////////////////////////////////////////////////////////
	// Attempt to open a file. Returns false on error.

	bool
	MMappedFile::mapFile(filename_str_t filename_, filename_str_t dirname_) MMAPPER_MAYBE_NOEXCEPT
	{
		// Release any file we currently have open.
		unmapFile();

		// New filename.	
		m_filename = _populateFilename(filename_, dirname_);

		FileHandle fh{ m_filename };
		if (!fh.isValid())
			return false;

		auto size = fh.uncachedFileSize();
		if (!size)
		{
	#ifndef MMAPPER_NO_THROW
			throw std::runtime_error("trying to map zero-sized file");
	#endif
			return false;
		}

		// Ask the OS to provide an in-memory view of the data; which is
		// basically saying "load this file into buffers like you would,
		// but then give us direct access to the buffer memory".
	#if MMAPPER_API == MMAPPER_WIN32
		// Windows implementation.
		FileHandle mapFh{ CreateFileMapping(fh, NULL, PAGE_READONLY, 0, 0, NULL) };
		if (!mapFh.isValid())
		{
	#ifndef MMAPPER_NO_THROW
			throw std::runtime_error("Failed to create file mapping");
	#endif
			return false;
		}

		// We don't need the original 'fh' now, so we can go ahead and close it,
		// the MapView call can then potentially re-use it and avoid some
		// allocations.
		fh.close();

		LPVOID const ptr = MapViewOfFileEx(mapFh, FILE_MAP_READ, 0, 0, 0, NULL);
		constexpr LPVOID MapFailure = nullptr;
	#else
		// POSIX implementation.
		constexpr int flags = 
				  MAP_FILE 		// Compatibility flag, not really required.
				| MAP_SHARED	// Share buffers with any other mmappers of this file.
				;
		void* const ptr = mmap(NULL, size + 1, PROT_READ, flags, fh, 0);
		constexpr void* MapFailure = MAP_FAILED;
	#endif

		if (ptr == MapFailure)
		{
	#ifndef MMAPPER_NO_THROW
			throw std::runtime_error("Mapping failed");
	#endif
			return false;
		}

		// Both methods return a pointer to the beginning of the OS'es internal
		// buffers. There may not be any data there, and the first access may
		// result in a page fault (the OS has to actually fetch data, akin to the
		// first call of read()).
		m_basePtr = static_cast<void*>(ptr);

		// For convenience, pre-calculate where the end of the data is.
		m_endPtr = static_cast<const char*>(ptr) + size;

		// All the file handles we have open at this point are now safe to close.

		return true;
	}


	//////////////////////////////////////////////////////////////////////
	// Close and unmap the mapping.

	bool MMappedFile::unmapFile() MMAPPER_MAYBE_NOEXCEPT
	{
		if (!isMapped())
		{
	#ifndef MMAPPER_NO_THROW
			throw std::logic_error("File is already unmapped.");
	#endif
			return false;
		}

	#if MMAPPER_API == MMAPPER_WIN32
		UnmapViewOfFile(m_basePtr);
	#else
		size_t size = m_endPtr - m_basePtr + 1;
		munmap(m_basePtr, size);
	#endif

		m_filename.clear();
		m_basePtr = nullptr;
		m_endPtr = nullptr;

		return true;
	}

}
