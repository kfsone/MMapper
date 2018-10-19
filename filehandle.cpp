// mmapper filehandle raii container
// Author: Oliver "kfsone" Smith 2018 <oliver@kfs.org>
// Redistribution and re-use fully permitted contingent on inclusion of these 3 lines in copied- or derived- works.


#include "filehandle.h"
#include "internal_includes.h"

#include <utility>

#ifndef O_BINARY
#define O_BINARY 0
#endif

namespace KFS
{

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	FileHandle::FileHandle(const filename_str_t& filename_) MMAPPER_MAYBE_NOEXCEPT
	{
#if MMAPPER_API == MMAPPER_WIN32
		// Windows implementation.
		m_fd = CreateFile(filename_.c_str(), FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
#else
		m_fd = open(filename_.c_str(), O_RDONLY | O_BINARY);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Track a descriptor someone else opened.

	FileHandle::FileHandle(file_handle_t fd_) MMAPPER_MAYBE_NOEXCEPT
		: m_fd(fd_)
	{
#ifndef MMAPPER_NO_THROW
		if (fd_ == INVALID_HANDLE_VALUE)
			throw std::logic_error("Passed invalid handle to FileHandle()");
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// Destructor, guarantee any open file is closed.

	FileHandle::~FileHandle() noexcept
	{
		// Doing the validity check here ensures we don't throw.
		if (isValid())
			close();
	}


	//////////////////////////////////////////////////////////////////////////
	// Close the descriptor if it's open.

	void FileHandle::close() MMAPPER_MAYBE_NOEXCEPT
	{
		// When throws are enabled, make user aware of invalid close actions.
		// If it's not defined, there's no side effect to an invalid close.
		if (!isValid())
		{
#ifndef MMAPPER_NO_THROW
			throw std::logic_error("Tried to close an invalid/closed FileHandle");
#endif
			return;
		}

		// Invalidate the handle but keep the value so we can close it.
		file_handle_t fd = std::exchange(m_fd, INVALID_HANDLE_VALUE);

		// Close the descriptor we had.
#if MMAPPER_API == MMAPPER_WIN32
		CloseHandle(m_fd);
#else
		::close(m_fd);
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// Retrieve the file size if the file is open, but don't cache it.

	size_t FileHandle::uncachedFileSize() const MMAPPER_MAYBE_NOEXCEPT
	{
		// File must be open to give you a size.
		if (!isValid())
		{
#ifndef MMAPPER_NO_THROW
			throw std::logic_error("Tried to get size of unopened handle");
#endif
			return 0ULL;
		}

		size_t size{ 0ULL };
		// Determine file size.
#if MMAPPER_API == MMAPPER_WIN32
		LARGE_INTEGER quadSize{ 0 };
		if (!GetFileSizeEx(m_fd, &quadSize))
		{
	#ifndef MMAPPER_NO_THROW
			throw std::runtime_error("GetFileSizeEx failed");
	#endif
		}
		else
		{
			size = (size_t)quadSize.QuadPart;
		}

#else

		// Use fstat to quickly obtain the file size.
		struct stat stats;
		const int sr = fstat(m_fd, &stats);
		if (sr < 0)
		{
	#ifndef MMAPPER_NO_THROW
			throw std::runtime_error("fstat failed");
	#endif
		}
		else
		{
			size = stats.st_size;
		}
#endif

		return size;
	}
};

