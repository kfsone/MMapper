#pragma once

// MMapper -- Cross-platform (Win/Posix) mmap interface.
// Author: Oliver "kfsone" Smith 2012, 2018 <oliver@kfs.org>
// Redistribution and re-use fully permitted contingent on inclusion of these 3 lines in copied- or derived- works.

#include "mmapper_platform.h"

namespace KFS
{

	//////////////////////////////////////////////////////////////////////
	//! @class MMapper
	//! @brief Provides an interface for basic memory-mapped file access
	//! on Windows, Mac and Linux/POSIX platforms.
	//!
	//! @detail Normalizes the Windows and Linux memory-mapped file
	//! APIs into a single, RAII interface that works cross platform.
	//!
	//! Normally, read() has to have the OS load data from disk into
	//! private buffers and then has to copy that data into user-space
	//! memory (the buffers you supply). 
	//!
	//! Mapping the file effectively gives you direct access to those
	//! underlying buffers so that you can access the data without
	//! copying.
	//!
	//! Benefits:
	//!  - Reduces memory pressure (no copies),
	//!  - Increases cache coherency (file data in a single location),
	//!  - Reduces memory usage/management overhead,
	//!  - Works with the OSes disk-buffer management rather than against.
	//


	class MMappedFile
	{
		//! Name of the file.
		filename_str_t	m_filename{};

		//! Where the file contents actually starts.
		const void*		m_basePtr{ nullptr };

		//! For convenience, where the file would end.
		const char*		m_endPtr{ nullptr };

	public:
		//! Simple default CTor.
		MMappedFile() noexcept = default;

		//! Create an in-memory view of a file from disk, while trying to avoid
		//! loading it from disk.
		//!
		//! @param[in] filename_ the file/sub-path of the file to open.
		//! @param[in] dirname_ [optional] prefix path.
		MMappedFile(filename_str_t filename_, filename_str_t dirname = filename_str_t{}) MMAPPER_MAYBE_NOEXCEPT;

		// DTor.
		virtual ~MMappedFile() noexcept;

		// Copying not allowed. ///TODO: Consider using shared_ptr to make copyable.
		MMappedFile(const MMappedFile& rhs) = delete;
		MMappedFile& operator = (const MMappedFile& rhs_) = delete;

		// Move allowed.
		MMappedFile(MMappedFile&& rhs_) noexcept = default;
		MMappedFile& operator = (MMappedFile&& rhs_) noexcept = default;

	public:
		//! Open a new file (closes any currently open file first).
		//!
		//! @param[in] filename_ the file/sub-path of the file to open.
		//! @param[in] dirname_ [optional] prefix path.
		//!
		//! @return true if the file was opened, false otherwise.
		bool mapFile(filename_str_t filename_, filename_str_t dirname_ = filename_str_t{}) MMAPPER_MAYBE_NOEXCEPT;

		//! Release the mapping of the file.
		//! @return true on success, or false/throw if the file is already unmapped.
		bool unmapFile() MMAPPER_MAYBE_NOEXCEPT;

		//////////////////////////////////////////////////////////////////////
		// Accessors.

		//! Check if we are not currently mapping anything.
		//! @return true when nothing is mapped, otherwise false.
		bool isMapped() const noexcept { return m_basePtr != nullptr; }

		//! Return the current file name, if any.
		const filename_str_t& filename() const noexcept { return m_filename; }

		//! Get a pointer to the base of the memory view, or NULL.
		template<typename T=char>
		const T* begin() const noexcept { return reinterpret_cast<const T*>(m_basePtr); }

		//! Pointer to EOF (last byte + 1
		template<typename T=char>
		const T* end()  const noexcept  { return reinterpret_cast<const T*>(m_endPtr); }

		size_t size() const noexcept { return end() - begin(); }
	};

}
