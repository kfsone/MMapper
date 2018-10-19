#pragma once

// MMapper -> FileHandle -- Cross-platform (Win/Posix) mmap interface.
// Author: Oliver "kfsone" Smith 2012, 2018 <oliver@kfs.org>
// Redistribution and re-use fully permitted contingent on inclusion of these 3 lines in copied- or derived- works.

#include "mmapper_platform.h"


namespace KFS
{

	//////////////////////////////////////////////////////////////////////
	// Helper that tracks a file handle and ensures it closes if we
	// have to bail.
	class FileHandle final
	{
		// The all-important file handle type.
		file_handle_t	m_fd{ INVALID_HANDLE_VALUE };

	public:
		//! Filename ctor: Open the named file and track the file handle.
		FileHandle(const filename_str_t& filename_) MMAPPER_MAYBE_NOEXCEPT;

		//! Handle ctor: Track an already open file handle.
		//! Passing the invalid handle will raise an exception unless MMAPPER_NO_THROW is defined.
		explicit FileHandle(file_handle_t fd_) MMAPPER_MAYBE_NOEXCEPT;

		// Default and copy-construction disabled.
		FileHandle() = delete;
		FileHandle(const FileHandle&) = delete;
		FileHandle& operator=(const FileHandle&) = delete;

		// Move allowed.
		FileHandle(FileHandle&&) noexcept = default;
		FileHandle& operator=(FileHandle&&) noexcept = default;

		// Destructor: close the file.
		~FileHandle() noexcept;

		void close() MMAPPER_MAYBE_NOEXCEPT;

		//! Boolean check whether this filehandle represents an open file.
		//! @return true if we have an open file, else false.
		bool isValid() const noexcept { return m_fd; }

		operator file_handle_t () const noexcept { return m_fd; }

		//! Perform a stat-type operation to retrieve the size of an open file.
		//! No attempt is made to cache the value, so repeated calls may result
		//! in multiple system calls.
		//!
		//! @return size of the file or 0ULL if an error occurred in no throw mode.
		size_t uncachedFileSize() const MMAPPER_MAYBE_NOEXCEPT;
	};

}