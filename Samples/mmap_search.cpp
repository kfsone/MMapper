//////////////////////////////////////////////////////////////////////
// MMapper Common Demo -- simple demonstration of using KFS::MMapper
// Author: Oliver "kfsone" Smith 2012 <oliver@kfs.org>
// Redistribution and re-use fully permitted contingent on inclusion of these 3 lines in copied- or derived- works.
//////////////////////////////////////////////////////////////////////
// Simple demonstration of using KFS::MMapper to read a text file.
//
// Command line only, usage:
//
//  common_demo <word> <filename1> [... <filenameN>]
//
// Case-sensitive search for 'word' in the listed files.


#include "mmapper.h"			// For KFS::MMappedFile
#include <iostream>				// For std::cout, cerr, endl, etc.
#include <stdexcept>			// For std::exception types.
#include <cstring>				// For strstr.


int	main(int argc, const char* const argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <word> <filename1> [... <filenameN>]" << std::endl;
		std::cerr << "Performs a case-sensitive search for 'word' in the listed files using memory-mapped IO." << std::endl;
		return 1;
	}

	// The files are the haystack, the word is the needle, a searching we shall go.
	const char* const needle = argv[1];
	if (needle == nullptr || *needle == 0)
	{
		std::cerr << "Very clever, you passed me an empty word to search for. Very clever." << std::endl;
		return 2;
	}

	for (size_t fileNo = 2; fileNo < static_cast<size_t>(argc); ++fileNo)
	{
		const char* const filename = argv[fileNo];
		try
		{
			KFS::MMappedFile mf(argv[fileNo]);

			// We can use good-old strstr because MappedFile ensures a
			// terminating \0 at the end of the allocation.
			const char* begin = static_cast<const char*>(mf.begin());
			const char* const location = strstr(begin, needle);
			if (location != NULL && *location != 0)
			{
				std::cout << filename << " matches." << std::endl;
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "ERROR:" << filename << ": " << e.what() << std::endl;
		}
	}

	return 0;
}
